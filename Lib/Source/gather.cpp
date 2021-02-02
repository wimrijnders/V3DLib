#include "gather.h"


namespace V3DLib {

Stmt::Ptr gatherExpr(Expr::Ptr e) {
  return Stmt::create_assign(mkVar(Var(TMU0_ADDR)), e);
}


/**
 * Pre: param is pointer type
 */
void gatherBaseExpr(BaseExpr const &addr) {
  if (Platform::instance().compiling_for_vc4()) {
    // Intention:
    // Pointer temp = addr + index();
    Int b = index();
    Expr::Ptr temp = mkApply(addr.expr(), Op(ADD, INT32), (b << 2).expr());

    stmtStack() << gatherExpr(temp);
  } else {
    stmtStack() <<  gatherExpr(addr.expr());
  }
}


void receiveExpr(Expr::Ptr e) {
  stmtStack() << Stmt::create(Stmt::LOAD_RECEIVE, e, nullptr);
}

void receive(Int &dest)   { receiveExpr(dest.expr()); }
void receive(Float &dest) { receiveExpr(dest.expr()); }

}  // namespace V3DLib
