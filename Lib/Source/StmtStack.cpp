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
  auto assign = Stmt::create_assign(mkVar(Var(TMU0_ADDR)), exp.expr());

  if (preload->tag == Stmt::GATHER_PRELOAD) {
    assign->comment("Start Preload");
  }

  preload->append(assign);
  std::cout << preload->dump() << std::endl;
}


void StmtStack::add_preload(V3DLib::Ptr<Int> &src) {
  add_preload((BaseExpr const &) src );
}


StmtStack &stmtStack() {
  assert(p_stmtStack != nullptr);
  return *p_stmtStack;
}


void clearStack() {
  assert(p_stmtStack != nullptr);
  p_stmtStack = nullptr;
}


void setStack(StmtStack &stmtStack) {
  assert(p_stmtStack == nullptr);
  p_stmtStack = &stmtStack;
}

void add_preload(BaseExpr const &exp) {
  stmtStack().add_preload(exp);
}

}  // namespace V3DLib
