#include "DMA.h"
#include "Source/StmtStack.h"


namespace V3DLib {
namespace  {

//=============================================================================
// VPM Setup
//=============================================================================

Stmt *vpmSetupReadCore(int n, IntExpr addr, bool hor, int stride) {
  Stmt *s = Stmt::create(SETUP_VPM_READ, addr.expr(), nullptr);
  s->setupVPMRead.numVecs = n;
  s->setupVPMRead.stride = stride;
  s->setupVPMRead.hor = hor;

	return s;
}

Stmt *vpmSetupWriteCore(IntExpr addr, bool hor, int stride) {
  Stmt *s = Stmt::create(SETUP_VPM_WRITE, addr.expr(), nullptr);
  s->setupVPMWrite.stride = stride;
  s->setupVPMWrite.hor = hor;

	return s;
}

}  // anon namespace


void vpmPutExpr(Expr::Ptr e) {
  Stmt* s = mkAssign(mkVar(Var(VPM_WRITE)), e);
  stmtStack().append(s);
}


void dmaStartReadExpr(Expr::Ptr e) {
  Stmt* s = Stmt::create(DMA_START_READ, e, nullptr);
  stmtStack().append(s);
}


void dmaStartWriteExpr(Expr::Ptr e) {
  Stmt* s = Stmt::create(DMA_START_WRITE, e, nullptr);
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
  Stmt *s = Stmt::create(SET_READ_STRIDE, stride.expr(), nullptr);
  stmtStack() << s;
}

void dmaSetWriteStride(IntExpr stride) {
  Stmt *s = Stmt::create(SET_WRITE_STRIDE, stride.expr(), nullptr);
  stmtStack() << s;
}


void dmaSetupRead(Dir dir, int numRows, IntExpr vpmAddr, int rowLen, int vpitch) {
  Stmt *s = Stmt::create(SETUP_DMA_READ, vpmAddr.expr(), nullptr);
  s->setupDMARead.hor = dir == HORIZ ? 1 : 0;
  s->setupDMARead.numRows = numRows;
  s->setupDMARead.rowLen = rowLen;
  s->setupDMARead.vpitch = vpitch;

  stmtStack() << s;
}

void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, int rowLen) {
  Stmt *s = Stmt::create(SETUP_DMA_WRITE, vpmAddr.expr(), nullptr);
  s->setupDMAWrite.hor = dir == HORIZ ? 1 : 0;
  s->setupDMAWrite.numRows = numRows;
  s->setupDMAWrite.rowLen = rowLen;

  stmtStack() << s;
}

void dmaWaitRead() {
  Stmt *s = Stmt::create(DMA_READ_WAIT);
  stmtStack().append(s);
}

void dmaWaitWrite() {
  Stmt *s = Stmt::create(DMA_WRITE_WAIT);
  stmtStack().append(s);
}


//=============================================================================
// Semaphore access
//=============================================================================

void semaInc(int semaId) {
  Stmt *s = Stmt::create(SEMA_INC);
  s->semaId = semaId;
  stmtStack().append(s);
}

void semaDec(int semaId) {
  Stmt *s = Stmt::create(SEMA_DEC);
  s->semaId = semaId;
  stmtStack().append(s);
}


//=============================================================================
// Host IRQ
//=============================================================================

void hostIRQ() {
  Stmt *s = Stmt::create(SEND_IRQ_TO_HOST);
  stmtStack().append(s);
}

}  // namespace V3DLib
