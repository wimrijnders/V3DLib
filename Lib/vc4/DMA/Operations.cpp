#include "Operations.h"
#include "Source/StmtStack.h"

namespace V3DLib {
namespace  {

//=============================================================================
// VPM Setup
//=============================================================================

Stmt::Ptr vpmSetupReadCore(int n, IntExpr addr, bool hor, int stride) {
  Stmt::Ptr s = Stmt::create(Stmt::SETUP_VPM_READ);
  s->dma.setupVPMRead(n, addr.expr(), hor, stride);
  return s;
}


Stmt::Ptr vpmSetupWriteCore(IntExpr addr, bool hor, int stride) {
  Stmt::Ptr s = Stmt::create(Stmt::SETUP_VPM_WRITE);
  s->dma.setupVPMWrite(addr.expr(), hor, stride);
  return s;
}

}  // anon namespace


void vpmPutExpr(Expr::Ptr e) {
  stmtStack() << Stmt::create_assign(mkVar(Var(VPM_WRITE)), e);
}


void dmaStartReadExpr(Expr::Ptr e) {
  Stmt::Ptr s = Stmt::create(Stmt::DMA_START_READ, e, nullptr);
  stmtStack().append(s);
}


void dmaStartWriteExpr(Expr::Ptr e) {
  Stmt::Ptr s = Stmt::create(Stmt::DMA_START_WRITE, e, nullptr);
  stmtStack().append(s);
}


void vpmSetupRead(Dir d, int n, IntExpr addr, int stride) {
  stmtStack() << vpmSetupReadCore(n, addr, d == HORIZ ? 1 : 0, stride);
}


void vpmSetupWrite(Dir d, IntExpr addr, int stride) {
  stmtStack() << vpmSetupWriteCore(addr, d == HORIZ ? 1 : 0, stride);
}


// ============================================================================
// DMA
// ============================================================================

void dmaSetReadPitch(IntExpr stride) {
  stmtStack() << Stmt::create(Stmt::SET_READ_STRIDE, stride.expr(), nullptr);
}


void dmaSetWriteStride(IntExpr stride) {
  stmtStack() << Stmt::create(Stmt::SET_WRITE_STRIDE, stride.expr(), nullptr);
}


void dmaSetupRead(Dir dir, int numRows, IntExpr vpmAddr, int rowLen, int vpitch) {
  Stmt::Ptr s = Stmt::create(Stmt::SETUP_DMA_READ);
  s->dma.setupDMARead(dir == HORIZ, numRows, vpmAddr.expr(), rowLen, vpitch);
  stmtStack() << s;
}


void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, IntExpr rowLen) {
  Stmt::Ptr s = Stmt::create(Stmt::SETUP_DMA_WRITE);
  s->dma.setupDMAWrite(dir == HORIZ, numRows, vpmAddr.expr(), rowLen);
  stmtStack() << s;
}


void dmaWaitRead() {
  stmtStack() << Stmt::create(Stmt::DMA_READ_WAIT);
}


void dmaWaitWrite() {
  stmtStack() << Stmt::create(Stmt::DMA_WRITE_WAIT);
}


//=============================================================================
// Semaphore access
//=============================================================================

void semaInc(int semaId) {
  Stmt::Ptr s = Stmt::create(Stmt::SEMA_INC);
  s->dma.semaId(semaId);
  stmtStack() << s;
}

void semaDec(int semaId) {
  Stmt::Ptr s = Stmt::create(Stmt::SEMA_DEC);
  s->dma.semaId(semaId);
  stmtStack() << s;
}


//=============================================================================
// Host IRQ
//=============================================================================

void hostIRQ() {
  stmtStack() << Stmt::create(Stmt::SEND_IRQ_TO_HOST);
}

}  // namespace V3DLib
