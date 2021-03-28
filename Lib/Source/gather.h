#ifndef _V3DLIB_SOURCE_GATHER_H_
#define _V3DLIB_SOURCE_GATHER_H_
#include "Support/Platform.h"
#include "StmtStack.h"
#include "Ptr.h"
#include "Float.h"

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


inline void gather(Pointer &addr) {
  gatherBaseExpr(addr);
}


void receiveExpr(Expr::Ptr e);
void receive(Int &dest);
void receive(Float &dest);

inline void receive(BaseExpr &dest) { receiveExpr(dest.expr()); }


//=============================================================================
// Gather, receive with gather limit
//=============================================================================

void gather(Float::Ptr &addr_a, Float::Ptr &addr_b);
void receive(Float &dst, Float::Ptr &src);
void receive();

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_GATHER_H_
