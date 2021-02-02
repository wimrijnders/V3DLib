#ifndef _V3DLIB_VC4_DMA_OPERATIONS_H_
#define _V3DLIB_VC4_DMA_OPERATIONS_H_
#include "Source/Int.h"     // IntExpr
#include "Source/Ptr.h"
#include "Source/Stmt.h"    // stmtStack()

namespace V3DLib {

// Direction for VPM/DMA loads and stores
enum Dir { HORIZ, VERT };

//=============================================================================
// VPM operations
//=============================================================================

void vpmPutExpr(Expr::Ptr e);
void dmaStartReadExpr(Expr::Ptr e);
void dmaStartWriteExpr(Expr::Ptr e);

inline void vpmPut(IntExpr data)   { vpmPutExpr(data.expr()); }
inline void vpmPut(FloatExpr data) { vpmPutExpr(data.expr()); }

template <typename T>
inline void vpmPut(PtrExpr<T> data) { vpmPutExpr(data.expr()); }

template <typename T>
inline void vpmPut(Ptr<T> &data)    { vpmPutExpr(data.expr()); }

template <typename T>
inline void dmaStartRead(PtrExpr<T> memAddr) { dmaStartReadExpr(memAddr.expr()); }

template <typename T>
inline void dmaStartRead(Ptr<T> &memAddr)    { dmaStartReadExpr(memAddr.expr()); }

template <typename T>
inline void dmaStartWrite(PtrExpr<T> memAddr) { dmaStartWriteExpr(memAddr.expr()); }

template <typename T>
inline void dmaStartWrite(Ptr<T> &memAddr)    { dmaStartWriteExpr(memAddr.expr()); }

void vpmSetupRead(Dir dir, int n, IntExpr addr, int stride = 1);
void vpmSetupWrite(Dir dir, IntExpr addr, int stride = 1);
void dmaSetReadPitch(IntExpr n);
void dmaSetWriteStride(IntExpr n);
void dmaSetupRead(Dir dir, int numRows, IntExpr vpmAddr, int rowLen = 16, int vpitch = 1);
void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, int rowLen = 16);
void dmaWaitRead();
void dmaWaitWrite();
void semaInc(int semaId);
void semaDec(int semaId);
void hostIRQ();

}  // namespace V3DLib

#endif  // _V3DLIB_VC4_DMA_OPERATIONS_H_
