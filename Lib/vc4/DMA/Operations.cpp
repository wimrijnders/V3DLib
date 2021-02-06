#include "Operations.h"
#include "Source/StmtStack.h"

namespace V3DLib {
namespace  {

//=============================================================================
// VPM Setup
//=============================================================================

Stmt::Ptr vpmSetupReadCore(int n, IntExpr addr, bool hor, int stride) {
  Stmt::Ptr s = Stmt::create(Stmt::SETUP_VPM_READ, addr.expr(), nullptr);
  s->setupVPMRead.numVecs = n;
  s->setupVPMRead.stride = stride;
  s->setupVPMRead.hor = hor;

  return s;
}

Stmt::Ptr vpmSetupWriteCore(IntExpr addr, bool hor, int stride) {
  Stmt::Ptr s = Stmt::create(Stmt::SETUP_VPM_WRITE, addr.expr(), nullptr);
  s->setupVPMWrite.stride = stride;
  s->setupVPMWrite.hor = hor;

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
  Stmt::Ptr s = Stmt::create(Stmt::SETUP_DMA_READ, vpmAddr.expr(), nullptr);
  s->setupDMARead.hor = dir == HORIZ ? 1 : 0;
  s->setupDMARead.numRows = numRows;
  s->setupDMARead.rowLen = rowLen;
  s->setupDMARead.vpitch = vpitch;

  stmtStack() << s;
}


void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, int rowLen) {
  Stmt::Ptr s = Stmt::create(Stmt::SETUP_DMA_WRITE, vpmAddr.expr(), nullptr);
  s->setupDMAWrite.hor = dir == HORIZ ? 1 : 0;
  s->setupDMAWrite.numRows = numRows;
  s->setupDMAWrite.rowLen = rowLen;

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
  s->semaId = semaId;
  stmtStack() << s;
}

void semaDec(int semaId) {
  Stmt::Ptr s = Stmt::create(Stmt::SEMA_DEC);
  s->semaId = semaId;
  stmtStack() << s;
}


//=============================================================================
// Host IRQ
//=============================================================================

void hostIRQ() {
  stmtStack() << Stmt::create(Stmt::SEND_IRQ_TO_HOST);
}

}  // namespace V3DLib
