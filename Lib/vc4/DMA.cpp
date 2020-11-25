#include "DMA.h"


namespace V3DLib {

//=============================================================================
// VPM Setup
//=============================================================================

static void vpmSetupReadCore(int n, IntExpr addr, bool hor, int stride) {
  Stmt *s = Stmt::create(SETUP_VPM_READ);
  s->setupVPMRead.numVecs = n;
  s->setupVPMRead.stride = stride;
  s->setupVPMRead.hor = hor;
  s->setupVPMRead.addr = addr.expr;
  stmtStack() << s;
}

static void vpmSetupWriteCore(IntExpr addr, bool hor, int stride) {
  Stmt *s = Stmt::create(SETUP_VPM_WRITE);
  s->setupVPMWrite.stride = stride;
  s->setupVPMWrite.hor = hor;
  s->setupVPMWrite.addr = addr.expr;
  stmtStack().append(s);
}


void vpmSetupRead(Dir d, int n, IntExpr addr, int stride)
{
  vpmSetupReadCore(n, addr, d == HORIZ ? 1 : 0, stride);
}


void vpmSetupWrite(Dir d, IntExpr addr, int stride)
{
  vpmSetupWriteCore(addr, d == HORIZ ? 1 : 0, stride);
}


// ============================================================================
// DMA
// ============================================================================

void dmaSetReadPitch(IntExpr stride) {
  Stmt *s = Stmt::create(SET_READ_STRIDE);
  s->stride = stride.expr;
  stmtStack().append(s);
}

void dmaSetWriteStride(IntExpr stride) {
  Stmt *s = Stmt::create(SET_WRITE_STRIDE);
  s->stride = stride.expr;
  stmtStack().append(s);
}

void dmaSetupRead(Dir dir, int numRows, IntExpr vpmAddr, int rowLen, int vpitch) {
  Stmt *s = Stmt::create(SETUP_DMA_READ);
  s->setupDMARead.hor = dir == HORIZ ? 1 : 0;
  s->setupDMARead.numRows = numRows;
  s->setupDMARead.rowLen = rowLen;
  s->setupDMARead.vpitch = vpitch;
  s->setupDMARead.vpmAddr = vpmAddr.expr;
  stmtStack().append(s);
}

void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, int rowLen) {
  Stmt *s = Stmt::create(SETUP_DMA_WRITE);
  s->setupDMAWrite.hor = dir == HORIZ ? 1 : 0;
  s->setupDMAWrite.numRows = numRows;
  s->setupDMAWrite.rowLen = rowLen;
  s->setupDMAWrite.vpmAddr = vpmAddr.expr;
  stmtStack().append(s);
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
