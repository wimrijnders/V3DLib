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


Stmts tempStmt(StackCallback f) {
  StmtStack::Ptr assign = tempStack(f);
  assert(assign->size() == 1);
  return *assign->top();
}


///////////////////////////////////////////////////////////////////////////////
// Class StmtStack::PrefetchContext
///////////////////////////////////////////////////////////////////////////////

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
    auto &assigns = *assign->top();

    assert(!m_prefetch_tags.empty());
    m_prefetch_tags[0]->append(assigns);
  }

  for (int i = 1; i < (int) m_prefetch_tags.size(); ++i) {
    int assign_index = i + (Platform::gather_limit() - 1);

    if (assign_index >= (int) m_assigns.size()) {
      break;
    }

    assert(!m_prefetch_tags.empty());
    assert(m_assigns[assign_index]->size() == 1);

    auto &assigns = *m_assigns[assign_index]->top();
    m_prefetch_tags[i]->append(assigns);
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


void StmtStack::PrefetchContext::add_prefetch_label(Stmt::Ptr pre) {
  m_prefetch_tags.push_back(pre);
}


///////////////////////////////////////////////////////////////////////////////
// Class StmtStack
///////////////////////////////////////////////////////////////////////////////


void StmtStack::add_prefetch_label(int prefetch_label) {
  auto pre = Stmt::create(Stmt::GATHER_PREFETCH);
  append(pre);
  prefetches[prefetch_label].add_prefetch_label(pre);
}


void StmtStack::first_prefetch(int prefetch_label) {
  if (!prefetches[prefetch_label].tags_empty()) return;
  add_prefetch_label(prefetch_label);
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
  add_prefetch_label(prefetch_label);

  Ptr assign = tempStack([&exp] {
    Pointer ptr = exp;
    gatherBaseExpr(exp);
  });

  prefetches[prefetch_label].post_prefetch(assign);
}


void StmtStack::add_prefetch(Pointer &exp, int prefetch_label) {
  add_prefetch_label(prefetch_label);

  Ptr assign = tempStack([&exp] {
    gatherBaseExpr(exp);
    exp.inc();
  });

  prefetches[prefetch_label].post_prefetch(assign);
}


void StmtStack::resolve_prefetches() {
  for (auto &item : prefetches) {
    item.second.resolve_prefetches();
  }
}


void StmtStack::push(Stmt::Ptr s) {
  if (empty()) {
    push();
  }

  top()->push_back(s);
}


/**
 * Return the last added statement on the stack
 */
Stmt::Ptr StmtStack::last_stmt() {
  assert(!empty());
  auto level = Parent::top();
  assert(level != nullptr);
  assert(!level->empty());

  auto ptr = level->back();
  assert(ptr.get() != nullptr);

  return ptr;
}


/**
 * Create a new level in the stack
 */
void StmtStack::push() {
  Stack::Ptr stmts(new Stmts());
  Parent::push(stmts);
}


void StmtStack::reset() {
  clear();
  push();

  prefetches.clear();
}


/**
 * Add passed statement to the end of the current instructions
 */
void StmtStack::append(Stmt::Ptr stmt) {
  assert(stmt.get() != nullptr);
  if (empty()) {
    push();
  }
  
  Parent::top()->push_back(stmt);
}


void StmtStack::append(Stmts const &stmts) {
  for (int i = 0; i < (int) stmts.size(); i++) {
    append(stmts[i]);
  }
}


std::string StmtStack::dump() const {
  using ::operator<<;  // C++ weirdness

  std::string ret;

  each([&ret] (Stmts const &item) {
    for (int i = 0; i < (int) item.size(); i++) {
      ret << item[i]->dump() << "\n";
    }
  });

  return ret;
}


/**
 * Only first item on stack is checked
 */
Stmt *StmtStack::first_in_seq() const {
  breakpoint  // TODO is this ever called?

  if (empty()) {
    return nullptr;
  }

  auto item = top();
  assert(item.get() != nullptr);
  assert(!item->empty());
  return (*item)[0]->first_in_seq();
}


StmtStack &stmtStack() {
  assert(p_stmtStack != nullptr);
  return *p_stmtStack;
}


void clearStack() {
  if (p_stmtStack == nullptr) {  // May occur if error during initialization
    return;
  }

  p_stmtStack->resolve_prefetches();
  p_stmtStack = nullptr;
}


void initStack(StmtStack &stmtStack) {
  assert(p_stmtStack == nullptr);
  stmtStack.reset();
  p_stmtStack = &stmtStack;
}


/**
 * Generate a new prefetch label
 */
int prefetch_label() {
  static int count = 0;
  return ++count;
}

}  // namespace V3DLib
