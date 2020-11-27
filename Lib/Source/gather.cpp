#include "gather.h"


namespace V3DLib {
namespace {

void storeExpr(Expr::Ptr e0, Expr::Ptr e1) {
  Stmt* s = Stmt::create(STORE_REQUEST, e0, e1);
  stmtStack() << s;
}

}  // anon namespace

void receiveExpr(Expr::Ptr e) {
  Stmt* s = Stmt::create(LOAD_RECEIVE);
  s->loadDest = e;
  stmtStack().append(s);
}

void receive(Int &dest)   { receiveExpr(dest.expr()); }
void receive(Float &dest) { receiveExpr(dest.expr()); }

void store(IntExpr data, PtrExpr<Int> addr)     { storeExpr(data.expr(), addr.expr()); }
void store(FloatExpr data, PtrExpr<Float> addr) { storeExpr(data.expr(), addr.expr()); }
void store(IntExpr data, Ptr<Int> &addr)        { storeExpr(data.expr(), addr.expr()); }
void store(FloatExpr data, Ptr<Float> &addr)    { storeExpr(data.expr(), addr.expr()); }

}  // namespace V3DLib
