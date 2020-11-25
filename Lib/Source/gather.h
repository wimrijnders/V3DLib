#ifndef _V3DLIB_SOURCE_GATHER_H_
#define _V3DLIB_SOURCE_GATHER_H_
#include "Support/Platform.h"
#include "Common/StmtStack.h"

namespace V3DLib {

//=============================================================================
// Receive, request, store operations
//=============================================================================

inline void gatherExpr(Expr* e)
{
  Var v; v.tag = TMU0_ADDR;
  Stmt* s = mkAssign(mkVar(v), e);
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

template <typename T>
inline void gather(PtrExpr<T> addr) {
	if (Platform::instance().compiling_for_vc4()) {
		Ptr<T> temp = addr + index();
		gatherExpr(temp.expr);
	} else {
		gatherExpr(addr.expr);
	}
}

template <typename T>
inline void gather(Ptr<T>& addr) {
	if (Platform::instance().compiling_for_vc4()) {
		Ptr<T> temp = addr + index();
		gatherExpr(temp.expr);
	} else {
		gatherExpr(addr.expr);
	}
}

inline void receiveExpr(Expr* e)
{
  Stmt* s = Stmt::create(LOAD_RECEIVE);
  s->loadDest = e;
  stmtStack().append(s);
}

inline void receive(Int& dest)
  { receiveExpr(dest.expr); }

inline void receive(Float& dest)
  { receiveExpr(dest.expr); }

template <typename T> inline void receive(Ptr<T>& dest)
  { receiveExpr(dest.expr); }


inline void storeExpr(Expr* e0, Expr* e1) {
  Stmt* s = Stmt::create(STORE_REQUEST, e0, e1);
  stmtStack().append(s);
}

inline void store(IntExpr data, PtrExpr<Int> addr) {
	storeExpr(data.expr, addr.expr);
}

inline void store(FloatExpr data, PtrExpr<Float> addr) {
	storeExpr(data.expr, addr.expr);
}

inline void store(IntExpr data, Ptr<Int> &addr) {
	storeExpr(data.expr, addr.expr);
}

inline void store(FloatExpr data, Ptr<Float> &addr) {
	storeExpr(data.expr, addr.expr);
}

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_GATHER_H_
