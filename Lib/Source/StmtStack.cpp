#include "StmtStack.h"
#include <iostream>          // std::cout
#include "Support/basics.h"
#include "Source/gather.h"

namespace V3DLib {
namespace {

StmtStack *p_stmtStack = nullptr;

StmtStack::Ptr tempStack(StackCallback f) {
  StmtStack::Ptr stack;
  stack.reset(new StmtStack);
  stack->reset();

  // Temporarily replace global stack
  StmtStack *global_stack = p_stmtStack;
  p_stmtStack = stack.get();

  f();  

  p_stmtStack = global_stack;
  return stack;
}

} // anon namespace


Stmt::Ptr tempStmt(StackCallback f) {
  StmtStack::Ptr assign = tempStack(f);
  assert(assign->size() == 1);
  return assign->top();
}


///////////////////////////////////////////////////////////////////////////////
// Class StmtStack::PrefetchContext
///////////////////////////////////////////////////////////////////////////////

void StmtStack::PrefetchContext::reset() {
  assert(m_assigns.empty());  // Should have been emptied beforehand
  m_assigns.clear();
  prefetch_count = 0;
}


void StmtStack::PrefetchContext::resolve_prefetches() {
  if (m_prefetch_tags.empty()) {
    return;  // nothing to do
  }
  assert(m_prefetch_tags.size() == m_assigns.size() + 1);  // One extra for the initial prefetch tag

  // first prefetches go to first tag
  for (int i = 0; i < Platform::gather_limit() &&  i < (int) m_assigns.size(); ++i) {
    auto assign = m_assigns[i];

/*
  std::cout << "===== assign stack =====\n"
            << assign->dump()
            << "========================\n"
            << std::endl;
*/

    assert(assign->size() == 1);
    m_prefetch_tags[0]->append(assign->top());
  }

  for (int i = 1; i < (int) m_prefetch_tags.size(); ++i) {
    int assign_index = i + (Platform::gather_limit() - 1);

    if (assign_index >= (int) m_assigns.size()) {
      break;
    }

    assert(m_assigns[assign_index]->size() == 1);
    m_prefetch_tags[i]->append(m_assigns[assign_index]->top());
  }

  m_prefetch_tags.clear();
  m_assigns.clear();

  assertq(empty(), "Still prefetch assigns present after compile");
}


void StmtStack::PrefetchContext::post_prefetch(Ptr assign) {
  assert(assign.get() != nullptr);
  assert(assign->size() == 1);  // Not expecting anything else
  assert(prefetch_count <= Platform::gather_limit());

/*
  std::cout << "===== assign stack =====\n"
            << assign->dump()
            << "========================\n"
            << std::endl;
*/

  if (m_prefetch_tags.empty()) {
    auto item = assign->first_in_seq();
    assert(item != nullptr);
    item->comment("Start Prefetch");
  }

  m_assigns.push_back(assign);
}


///////////////////////////////////////////////////////////////////////////////
// Class StmtStack
///////////////////////////////////////////////////////////////////////////////


void StmtStack::add_prefetch_label() {
  auto pre = Stmt::create(Stmt::GATHER_PREFETCH);
  append(pre);
  prefetch.add_prefetch_label(pre);
}


void StmtStack::first_prefetch(int prefetch_label) {
  if (!prefetch.m_prefetch_tags.empty()) return;
  add_prefetch_label();
}


/**
 * Technically, the param should be a pointer in some form.
 * This is not enforced right now.
 * TODO find a way to enforce this
 *
 * @return true if param added to prefetch list,
 *         false otherwise (prefetch list is full)
 */
void StmtStack::add_prefetch(PointerExpr const &exp, int prefetch_label) {
  add_prefetch_label();

  Ptr assign = tempStack([&exp] {
    Pointer ptr = exp;
    gatherBaseExpr(exp);
  });

  prefetch.post_prefetch(assign);
}


void StmtStack::add_prefetch(Pointer &exp, int prefetch_label) {
  add_prefetch_label();

  Ptr assign = tempStack([&exp] {
    gatherBaseExpr(exp);
    exp += 16;
  });

  prefetch.post_prefetch(assign);
}


void StmtStack::reset() {
  clear();
  push(mkSkip());
  prefetch.reset();
}


/**
 * Add passed statement to the end of the current instructions
 *
 * This is a logical operation;
 * a new sequence item is added on top, with the previous tree as the left branch,
 * and the new item as the right branch.
 *
 * The net effect is that the passed instruction is added to the end of the sequence
 * of statements to be compiled.
 */
void StmtStack::append(Stmt::Ptr stmt) {
  assert(stmt.get() != nullptr);
  assert(!empty());
  
  //push(Stmt::create_sequence(pop(), stmt));
  auto top = top_item();
  auto seq = Stmt::create_sequence(top->head, stmt);
  top->head = seq;
}


std::string StmtStack::dump() const {
  using ::operator<<;  // C++ weirdness

  std::string ret;

  each([&ret] (Stmt const & item) {
    ret << item.dump() << "\n";
  });

  return ret;
}


/**
 * Only first item on stack is checked
 */
Stmt *StmtStack::first_in_seq() const {
  if (empty()) {
    return nullptr;
  }

  Stmt::Ptr item = top();
  return item->first_in_seq();
}


StmtStack &stmtStack() {
  assert(p_stmtStack != nullptr);
  return *p_stmtStack;
}


void clearStack() {
  assert(p_stmtStack != nullptr);

  p_stmtStack->resolve_prefetches();
  p_stmtStack = nullptr;
}


void initStack(StmtStack &stmtStack) {
  assert(p_stmtStack == nullptr);
  stmtStack.reset();
  p_stmtStack = &stmtStack;
}

/*
void prefetch(PointerExpr addr) {
  stmtStack().add_prefetch(addr);
}

void prefetch(Pointer &addr) {
  stmtStack().add_prefetch(addr);
}
*/

}  // namespace V3DLib
