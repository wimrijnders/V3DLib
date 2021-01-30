#include "gather.h"


namespace V3DLib {
namespace {

void storeExpr(Expr::Ptr e0, Expr::Ptr e1) {
  stmtStack() << Stmt::create(Stmt::STORE_REQUEST, e0, e1);
}

}  // anon namespace


void gatherExpr(Expr::Ptr e) {
  stmtStack() << Stmt::create_assign(mkVar(Var(TMU0_ADDR)), e);
}

void receiveExpr(Expr::Ptr e) {
  stmtStack() << Stmt::create(Stmt::LOAD_RECEIVE, e, nullptr);
}

void receive(Int &dest)   { receiveExpr(dest.expr()); }
void receive(Float &dest) { receiveExpr(dest.expr()); }

void store(IntExpr data, PtrExpr<Int> addr)     { storeExpr(data.expr(), addr.expr()); }
void store(FloatExpr data, PtrExpr<Float> addr) { storeExpr(data.expr(), addr.expr()); }
void store(IntExpr data, Ptr<Int> &addr)        { storeExpr(data.expr(), addr.expr()); }
void store(FloatExpr data, Ptr<Float> &addr)    { storeExpr(data.expr(), addr.expr()); }

}  // namespace V3DLib
