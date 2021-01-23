#include "StmtStack.h"
#include <iostream>          // std::cout
#include "Support/basics.h"

namespace V3DLib {

namespace {
  StmtStack *p_stmtStack = nullptr;
} // anon namespace


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
  push(Stmt::create_sequence(pop(), stmt));
}


std::string StmtStack::dump() const {
  using ::operator<<;  // C++ weirdness

  std::string ret;

  each([&ret] (Stmt const & item) {
    ret << item.dump() << "\n";
  });

  return ret;
}


StmtStack &stmtStack() {
  assert(p_stmtStack != nullptr);
  return *p_stmtStack;
}


void clearStack() {
  assert(p_stmtStack != nullptr);

//  std::cout << p_stmtStack->dump();
//  breakpoint

  p_stmtStack = nullptr;
}


void setStack(StmtStack &stmtStack) {
  assert(p_stmtStack == nullptr);
  p_stmtStack = &stmtStack;
}

}  // namespace V3DLib
