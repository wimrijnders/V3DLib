#ifndef _QPULIB_SOURCE_STMTEXTRA_H_
#define _QPULIB_SOURCE_STMTEXTRA_H_

namespace QPULib {

//=============================================================================
// Host IRQ
//=============================================================================

inline void hostIRQ()
{
  Stmt* s = mkStmt();
  s->tag = SEND_IRQ_TO_HOST;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

//=============================================================================
// Semaphore access
//=============================================================================

inline void semaInc(int semaId)
{
  Stmt* s = mkStmt();
  s->tag = SEMA_INC;
  s->semaId = semaId;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

inline void semaDec(int semaId)
{
  Stmt* s = mkStmt();
  s->tag = SEMA_DEC;
  s->semaId = semaId;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

//=============================================================================
// VPM operations
//=============================================================================

inline void vpmPutExpr(Expr* e)
{
  Var v; v.tag = VPM_WRITE;
  Stmt* s = mkAssign(mkVar(v), e);
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

inline void vpmPut(IntExpr data)
  { vpmPutExpr(data.expr); }

inline void vpmPut(FloatExpr data)
  { vpmPutExpr(data.expr); }

template <typename T> inline void vpmPut(PtrExpr<T> data)
  { vpmPutExpr(data.expr); }

template <typename T> inline void vpmPut(Ptr<T> &data)
  { vpmPutExpr(data.expr); }

inline void dmaStartReadExpr(Expr* e)
{
  Stmt* s = mkStmt();
  s->tag = DMA_START_READ;
  s->startDMARead = e;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

template <typename T> inline void dmaStartRead(PtrExpr<T> memAddr)
  { dmaStartReadExpr(memAddr.expr); }

template <typename T> inline void dmaStartRead(Ptr<T> &memAddr)
  { dmaStartReadExpr(memAddr.expr); }

inline void dmaStartWriteExpr(Expr* e)
{
  Stmt* s = mkStmt();
  s->tag = DMA_START_WRITE; 
  s->startDMAWrite = e;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

template <typename T> inline void dmaStartWrite(PtrExpr<T> memAddr)
  { dmaStartWriteExpr(memAddr.expr); }

template <typename T> inline void dmaStartWrite(Ptr<T> &memAddr)
  { dmaStartWriteExpr(memAddr.expr); }
}  // namespace QPULib

#endif  // _QPULIB_SOURCE_STMTEXTRA_H_
