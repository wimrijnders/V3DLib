#include "StmtStack.h"
#include <iostream>          // std::cout
#include "Support/basics.h"
#include "Source/gather.h"

namespace V3DLib {

namespace {
  StmtStack *p_stmtStack = nullptr;
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


void StmtStack::add_preload(BaseExpr const &exp) {
//  if (preload.get() != nullptr) {
//    std::cout << "Pre: " << preload->dump() << std::endl;
//  }

  if (preload == nullptr) {
    auto pre = Stmt::create(Stmt::GATHER_PRELOAD);
    append(pre);
    preload = pre;
  }

  assert(preload.get() != nullptr);
  assert(preload->tag == Stmt::SEQ || preload->tag == Stmt::GATHER_PRELOAD);

  StackPtr assign = tempStack([&exp] {
    gatherBaseExpr(exp);
  });
  assert(assign.get() != nullptr);
  assert(assign->size() == 1);  // Not expecting anything else

  if (preload->tag == Stmt::GATHER_PRELOAD) {
    auto item = assign->first_in_seq();
    assert(item != nullptr);
    item->comment("Start Preload");
  }

  preload->append(assign->top());
  std::cout << preload->dump() << std::endl;
}


void StmtStack::add_preload(V3DLib::Ptr<Int> &src) {
  add_preload((BaseExpr const &) src );
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


StackPtr tempStack(StackCallback f) {
  StackPtr stack;
  stack.reset(new StmtStack);
  stack->reset();

  // Temporarily replace global stack
  StmtStack *global_stack = p_stmtStack;
  p_stmtStack = stack.get();

  f();  

  p_stmtStack = global_stack;
  return stack;
}

}  // namespace V3DLib
