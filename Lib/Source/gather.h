#ifndef _V3DLIB_SOURCE_GATHER_H_
#define _V3DLIB_SOURCE_GATHER_H_
#include "Support/Platform.h"
#include "StmtStack.h"
#include "Ptr.h"

namespace V3DLib {

//=============================================================================
// Gather and receive operations
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


//=============================================================================
// Gather, receive with gather limit
//=============================================================================

void gather(Ptr<Float> &addr_a, Ptr<Float> &addr_b);
void receive(Float &dst, Ptr<Float> &src);
void receive();

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_GATHER_H_
