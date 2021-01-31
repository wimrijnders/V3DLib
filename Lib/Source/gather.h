#ifndef _V3DLIB_SOURCE_GATHER_H_
#define _V3DLIB_SOURCE_GATHER_H_
#include "Support/Platform.h"
#include "StmtStack.h"
#include "Ptr.h"

namespace V3DLib {

//=============================================================================
// Receive, request, store operations
//=============================================================================

Stmt::Ptr gatherExpr(Expr::Ptr e);

void gatherBaseExpr(BaseExpr const &addr);

template <typename T>
inline void gather(PtrExpr<T> addr) {
  gatherBaseExpr(addr);
}


template <typename T>
inline void gather(Ptr<T> &addr) {
  gatherBaseExpr(addr);
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
