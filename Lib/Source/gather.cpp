#include "gather.h"


namespace V3DLib {
namespace {

void storeExpr(Expr::Ptr e0, Expr::Ptr e1) {
  stmtStack() << Stmt::create(Stmt::STORE_REQUEST, e0, e1);
}

}  // anon namespace


Stmt::Ptr gatherExpr(Expr::Ptr e) {
  return Stmt::create_assign(mkVar(Var(TMU0_ADDR)), e);
}


/**
 * Pre: param is pointer type
 */
void gatherBaseExpr(BaseExpr const &addr) {
  if (Platform::instance().compiling_for_vc4()) {
    // Intention:
 		// Ptr<T> temp = addr + index();
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

void store(IntExpr data, PtrExpr<Int> addr)     { storeExpr(data.expr(), addr.expr()); }
void store(FloatExpr data, PtrExpr<Float> addr) { storeExpr(data.expr(), addr.expr()); }
void store(IntExpr data, Ptr<Int> &addr)        { storeExpr(data.expr(), addr.expr()); }
void store(FloatExpr data, Ptr<Float> &addr)    { storeExpr(data.expr(), addr.expr()); }

}  // namespace V3DLib
