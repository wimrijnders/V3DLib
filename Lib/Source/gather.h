#ifndef _QPULIB_SOURCE_GATHER_H_
#define _QPULIB_SOURCE_GATHER_H_

namespace QPULib {

//=============================================================================
// Receive, request, store operations
//=============================================================================

inline void gatherExpr(Expr* e)
{
  Var v; v.tag = TMU0_ADDR;
  Stmt* s = mkAssign(mkVar(v), e);
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

template <typename T> inline void gather(PtrExpr<T> addr)
  { gatherExpr(addr.expr); }

template <typename T> inline void gather(Ptr<T>& addr)
  { gatherExpr(addr.expr); }

inline void receiveExpr(Expr* e)
{
  Stmt* s = mkStmt();
  s->tag = LOAD_RECEIVE;
  s->loadDest = e;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

inline void receive(Int& dest)
  { receiveExpr(dest.expr); }

inline void receive(Float& dest)
  { receiveExpr(dest.expr); }

template <typename T> inline void receive(Ptr<T>& dest)
  { receiveExpr(dest.expr); }

inline void storeExpr(Expr* e0, Expr* e1)
{
  Stmt* s = mkStmt();
  s->tag = STORE_REQUEST;
  s->storeReq.data = e0;
  s->storeReq.addr = e1;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

inline void store(IntExpr data, PtrExpr<Int> addr)
  { storeExpr(data.expr, addr.expr); }

inline void store(FloatExpr data, PtrExpr<Float> addr)
  { storeExpr(data.expr, addr.expr); }

inline void store(IntExpr data, Ptr<Int> &addr)
  { storeExpr(data.expr, addr.expr); }

inline void store(FloatExpr data, Ptr<Float> &addr)
  { storeExpr(data.expr, addr.expr); }

}  // namespace QPULib

#endif  // _QPULIB_SOURCE_GATHER_H_
