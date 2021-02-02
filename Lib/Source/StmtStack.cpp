#include "StmtStack.h"
#include <iostream>          // std::cout
#include "Support/basics.h"
#include "Source/gather.h"

namespace V3DLib {

using StackCallback = std::function<void()>;

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


void StmtStack::reset() {
  clear();
  push(mkSkip());

  assert(m_assigns.empty());  // Should have been emptied beforehand
  m_assigns.clear();
  prefetch_count = 0;
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


void StmtStack::init_prefetch() {
  auto pre = Stmt::create(Stmt::GATHER_PREFETCH);
  append(pre);
  m_prefetch_tags.push_back(pre);
}


void StmtStack::first_prefetch() {
  if (!m_prefetch_tags.empty()) return;
  init_prefetch();
}


void StmtStack::post_prefetch(Ptr assign) {
  assert(assign.get() != nullptr);
  assert(assign->size() == 1);  // Not expecting anything else
  assert(prefetch_count <= 8);

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


/**
 * Technically, the param should be a pointer in some form.
 * This is not enforced right now.
 * TODO find a way to enforce this
 *
 * @return true if param added to prefetch list,
 *         false otherwise (prefetch list is full)
 */
void StmtStack::add_prefetch(PointerExpr const &exp) {
  init_prefetch();

  Ptr assign = tempStack([&exp] {
    Pointer ptr = exp;
    gatherBaseExpr(exp);
  });

  post_prefetch(assign);
}


void StmtStack::add_prefetch(Pointer &exp) {
  init_prefetch();

  Ptr assign = tempStack([&exp] {
    gatherBaseExpr(exp);
    exp += 16;
  });

  post_prefetch(assign);
}


void StmtStack::resolve_prefetches() {
  if (m_prefetch_tags.empty()) {
    return;  // nothing to do
  }
  assert(m_prefetch_tags.size() == m_assigns.size() + 1);  // One extra for the initial prefetch tag

  // first 8 prefetches go to first tag
  for (int i = 0; i < 8 &&  i < (int) m_assigns.size(); ++i) {
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
    int assign_index = i + (8 - 1);

    if (assign_index >= (int) m_assigns.size()) {
      break;
    }

    assert(m_assigns[assign_index]->size() == 1);
    m_prefetch_tags[i]->append(m_assigns[assign_index]->top());
  }

  m_prefetch_tags.clear();
  m_assigns.clear();
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
  assertq(p_stmtStack->prefetch_empty(), "Still prefetch assigns present after compile");
  p_stmtStack = nullptr;
}


void initStack(StmtStack &stmtStack) {
  assert(p_stmtStack == nullptr);
  stmtStack.reset();
  p_stmtStack = &stmtStack;
}


void prefetch(PointerExpr addr) {
  stmtStack().add_prefetch(addr);
}

void prefetch(Pointer &addr) {
  stmtStack().add_prefetch(addr);
}

}  // namespace V3DLib
