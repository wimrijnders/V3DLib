#ifndef _V3DLIB_SOURCE_GATHER_H_
#define _V3DLIB_SOURCE_GATHER_H_
#include "Support/Platform.h"
#include "StmtStack.h"
#include "Ptr.h"

namespace V3DLib {

//=============================================================================
// Receive, request, store operations
//=============================================================================

inline void gatherExpr(Expr::Ptr e) {
  stmtStack() << Stmt::create_assign(mkVar(Var(TMU0_ADDR)), e);
}


template <typename T>
inline void gather(PtrExpr<T> addr) {
	if (Platform::instance().compiling_for_vc4()) {
		Ptr<T> temp = addr + index();
		gatherExpr(temp.expr());
	} else {
		gatherExpr(addr.expr());
	}
}

template <typename T>
inline void gather(Ptr<T>& addr) {
	if (Platform::instance().compiling_for_vc4()) {
		Ptr<T> temp = addr + index();
		gatherExpr(temp.expr());
	} else {
		gatherExpr(addr.expr());
	}
}

void receiveExpr(Expr::Ptr e);
void receive(Int &dest);
void receive(Float &dest);

template <typename T>
inline void receive(Ptr<T> &dest) { receiveExpr(dest.expr); }

void store(IntExpr data, PtrExpr<Int> addr);
void store(FloatExpr data, PtrExpr<Float> addr);
void store(IntExpr data, Ptr<Int> &addr);
void store(FloatExpr data, Ptr<Float> &addr);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_GATHER_H_
