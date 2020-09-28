#include "DMA.h"


namespace QPULib {

//=============================================================================
// VPM Setup
//=============================================================================

static void vpmSetupReadCore(int n, IntExpr addr, bool hor, int stride)
{
  Stmt* s = mkStmt();
  s->tag = SETUP_VPM_READ;
  s->setupVPMRead.numVecs = n;
  s->setupVPMRead.stride = stride;
  s->setupVPMRead.hor = hor;
  s->setupVPMRead.addr = addr.expr;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

static void vpmSetupWriteCore(IntExpr addr, bool hor, int stride)
{
  Stmt* s = mkStmt();
  s->tag = SETUP_VPM_WRITE;
  s->setupVPMWrite.stride = stride;
  s->setupVPMWrite.hor = hor;
  s->setupVPMWrite.addr = addr.expr;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
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

void dmaSetReadPitch(IntExpr stride)
{
  Stmt* s = mkStmt();
  s->tag = SET_READ_STRIDE;
  s->stride = stride.expr;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

void dmaSetWriteStride(IntExpr stride)
{
  Stmt* s = mkStmt();
  s->tag = SET_WRITE_STRIDE;
  s->stride = stride.expr;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

void dmaSetupRead(Dir dir, int numRows, IntExpr vpmAddr,
                    int rowLen, int vpitch)
{
  Stmt* s = mkStmt();
  s->tag = SETUP_DMA_READ;
  s->setupDMARead.hor = dir == HORIZ ? 1 : 0;
  s->setupDMARead.numRows = numRows;
  s->setupDMARead.rowLen = rowLen;
  s->setupDMARead.vpitch = vpitch;
  s->setupDMARead.vpmAddr = vpmAddr.expr;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, int rowLen)
{
  Stmt* s = mkStmt();
  s->tag = SETUP_DMA_WRITE;
  s->setupDMAWrite.hor = dir == HORIZ ? 1 : 0;
  s->setupDMAWrite.numRows = numRows;
  s->setupDMAWrite.rowLen = rowLen;
  s->setupDMAWrite.vpmAddr = vpmAddr.expr;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

void dmaWaitRead()
{
  Stmt* s = mkStmt();
  s->tag = DMA_READ_WAIT;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

void dmaWaitWrite()
{
  Stmt* s = mkStmt();
  s->tag = DMA_WRITE_WAIT;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}


//=============================================================================
// Semaphore access
//=============================================================================

void semaInc(int semaId)
{
  Stmt* s = mkStmt();
  s->tag = SEMA_INC;
  s->semaId = semaId;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}

void semaDec(int semaId)
{
  Stmt* s = mkStmt();
  s->tag = SEMA_DEC;
  s->semaId = semaId;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}


//=============================================================================
// Host IRQ
//=============================================================================

void hostIRQ()
{
  Stmt* s = mkStmt();
  s->tag = SEND_IRQ_TO_HOST;
  stmtStack().replace(mkSeq(stmtStack().top(), s));
}


}  // namespace QPULib
