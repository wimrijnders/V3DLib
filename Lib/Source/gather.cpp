#include "gather.h"
#include "Support/Platform.h"

namespace V3DLib {

//=============================================================================
// Old School
//=============================================================================

Stmt::Ptr gatherExpr(Expr::Ptr e) {
  return Stmt::create_assign(mkVar(Var(TMU0_ADDR)), e);
}


/**
 * Pre: param is pointer type
 */
void gatherBaseExpr(BaseExpr const &addr) {
  if (Platform::compiling_for_vc4()) {
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


//=============================================================================
// With gather limit
//=============================================================================

namespace {

}  // anon namespace


void gather(Ptr<Float> &addr_a, Ptr<Float> &addr_b) {
  int count = Platform::gather_limit()/2;

  for (int i = 0; i < count; ++ i) {
    gatherBaseExpr(addr_a);
    addr_a.inc();
    gatherBaseExpr(addr_b);
    addr_b.inc();
  }
}


void receive(Float &dst, Ptr<Float> &src) {
  receive(dst);
  gatherBaseExpr(src);
  src.inc();
}


void receive() {
  int count = Platform::gather_limit();
  Int dummy;

  for (int i = 0; i < count; ++ i) {
    receive(dummy);
  }
}

}  // namespace V3DLib
