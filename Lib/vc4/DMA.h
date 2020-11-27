#ifndef _V3DLIB_SOURCE_DMA_H_
#define _V3DLIB_SOURCE_DMA_H_
#include "Source/Int.h"     // IntExpr
#include "Source/Ptr.h"
#include "Source/Syntax.h"  // Expr
#include "Common/StmtStack.h"
#include "Source/Stmt.h"    // stmtStack()

namespace V3DLib {

//=============================================================================
// VPM operations
//=============================================================================

inline void vpmPutExpr(Expr* e) {
  Stmt* s = mkAssign(mkVar(Var(VPM_WRITE)), e);
  stmtStack().append(s);
}

inline void vpmPut(IntExpr data)   { vpmPutExpr(data.expr()); }
inline void vpmPut(FloatExpr data) { vpmPutExpr(data.expr()); }

template <typename T>
inline void vpmPut(PtrExpr<T> data) { vpmPutExpr(data.expr()); }

template <typename T>
inline void vpmPut(Ptr<T> &data)    { vpmPutExpr(data.expr()); }

inline void dmaStartReadExpr(Expr* e) {
  Stmt* s = Stmt::create(DMA_START_READ, e, nullptr);
  stmtStack().append(s);
}

template <typename T> inline void dmaStartRead(PtrExpr<T> memAddr)
  { dmaStartReadExpr(memAddr.expr); }

template <typename T> inline void dmaStartRead(Ptr<T> &memAddr)
  { dmaStartReadExpr(memAddr.expr); }

inline void dmaStartWriteExpr(Expr* e) {
  Stmt* s = Stmt::create(DMA_START_WRITE, e, nullptr);
  stmtStack().append(s);
}

template <typename T> inline void dmaStartWrite(PtrExpr<T> memAddr)
  { dmaStartWriteExpr(memAddr.expr); }

template <typename T> inline void dmaStartWrite(Ptr<T> &memAddr)
  { dmaStartWriteExpr(memAddr.expr); }


void vpmSetupRead(Dir dir, int n, IntExpr addr, int stride = 1);
void vpmSetupWrite(Dir dir, IntExpr addr, int stride = 1);
void dmaSetReadPitch(IntExpr n);
void dmaSetWriteStride(IntExpr n);
void dmaSetupRead(Dir dir, int numRows, IntExpr vpmAddr,
                    int rowLen = 16, int vpitch = 1);
void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, int rowLen = 16);
void dmaWaitRead();
void dmaWaitWrite();
void semaInc(int semaId);
void semaDec(int semaId);
void hostIRQ();

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_DMA_H_
