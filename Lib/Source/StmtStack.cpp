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


bool StmtStack::init_prefetch() {
  assert(prefetch_count <= 8);
  if (prefetch_count >= 8) return false;

  if (prefetch == nullptr) {
    auto pre = Stmt::create(Stmt::GATHER_PREFETCH);
    append(pre);
    prefetch = pre;
  }

  assert(prefetch.get() != nullptr);
  assert(prefetch->tag == Stmt::SEQ || prefetch->tag == Stmt::GATHER_PREFETCH);
  return true;
}


void StmtStack::post_prefetch(Ptr assign) {
  assert(assign.get() != nullptr);
  assert(assign->size() == 1);  // Not expecting anything else

/*
  std::cout << "===== assign stack =====\n"
            << assign->dump()
            << "========================\n"
            << std::endl;
*/

  if (prefetch->tag == Stmt::GATHER_PREFETCH) {
    auto item = assign->first_in_seq();
    assert(item != nullptr);
    item->comment("Start Prefetch");
  }

  prefetch->append(assign->top());
  //std::cout << prefetch->dump() << std::endl;

  prefetch_count++;
}


/**
 * Technically, the param should be a pointer in some form.
 * This is not enforced right now.
 * TODO find a way to enforce this
 *
 * @return true if param added to prefetch list,
 *         false otherwise (prefetch list is full)
 */
bool StmtStack::add_prefetch(PointerExpr const &exp) {
  if (!init_prefetch()) return false;;

  Ptr assign = tempStack([&exp] {
    Pointer ptr = exp;
    gatherBaseExpr(exp);
  });

  post_prefetch(assign);
  return true;
}


bool StmtStack::add_prefetch(Pointer &exp) {
  if (!init_prefetch()) return false;;

  Ptr assign = tempStack([&exp] {
    gatherBaseExpr(exp);
    exp += 16;
  });

  post_prefetch(assign);
  return true;
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
  p_stmtStack = nullptr;
}


void initStack(StmtStack &stmtStack) {
  assert(p_stmtStack == nullptr);
  stmtStack.reset();
  p_stmtStack = &stmtStack;
}

}  // namespace V3DLib
