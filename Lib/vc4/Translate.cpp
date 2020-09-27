#include "Translate.h"
#include "Source/Translate.h"
#include "Target/LoadStore.h"

namespace QPULib {

namespace {

// ============================================================================
// Set-stride statements
// ============================================================================

void setStrideStmt(Seq<Instr>* seq, StmtTag tag, Expr* e)
{
  if (e->tag == INT_LIT) {
    if (tag == SET_READ_STRIDE)
      genSetReadPitch(seq, e->intLit);
    else
      genSetWriteStride(seq, e->intLit);
  }
  else if (e->tag == VAR) {
    if (tag == SET_READ_STRIDE)
      genSetReadPitch(seq, srcReg(e->var));
    else
      genSetWriteStride(seq, srcReg(e->var));
  }
  else {
    AssignCond always;
    always.tag = ALWAYS;
    Var v = freshVar();
    varAssign(seq, always, v, e);
    if (tag == SET_READ_STRIDE)
      genSetReadPitch(seq, srcReg(v));
    else
      genSetWriteStride(seq, srcReg(v));
  }
}

// ============================================================================
// VPM setup statements
// ============================================================================

void setupVPMReadStmt(Seq<Instr>* seq, int n, Expr* e, int hor, int stride)
{
  if (e->tag == INT_LIT)
    genSetupVPMLoad(seq, n, e->intLit, hor, stride);
  else if (e->tag == VAR)
    genSetupVPMLoad(seq, n, srcReg(e->var), hor, stride);
  else {
    AssignCond always;
    always.tag = ALWAYS;
    Var v = freshVar();
    varAssign(seq, always, v, e);
    genSetupVPMLoad(seq, n, srcReg(v), hor, stride);
  }
}

// ============================================================================
// DMA statements
// ============================================================================

void setupDMAReadStmt(Seq<Instr>* seq, int numRows, int rowLen,
                        int hor, Expr* e, int vpitch)
{
  if (e->tag == INT_LIT)
    genSetupDMALoad(seq, numRows, rowLen, hor, vpitch, e->intLit);
  else if (e->tag == VAR)
    genSetupDMALoad(seq, numRows, rowLen, hor, vpitch, srcReg(e->var));
  else {
    AssignCond always;
    always.tag = ALWAYS;
    Var v = freshVar();
    varAssign(seq, always, v, e);
    genSetupDMALoad(seq, numRows, rowLen, hor, vpitch, srcReg(v));
  }
}

void setupDMAWriteStmt(Seq<Instr>* seq, int numRows, int rowLen,
                        int hor, Expr* e)
{
  if (e->tag == INT_LIT)
    genSetupDMAStore(seq, numRows, rowLen, hor, e->intLit);
  else if (e->tag == VAR)
    genSetupDMAStore(seq, numRows, rowLen, hor, srcReg(e->var));
  else {
    AssignCond always;
    always.tag = ALWAYS;
    Var v = freshVar();
    varAssign(seq, always, v, e);
    genSetupDMAStore(seq, numRows, rowLen, hor, srcReg(v));
  }
}

void startDMAReadStmt(Seq<Instr>* seq, Expr* e)
{
  if (e->tag == VAR)
    genStartDMALoad(seq, srcReg(e->var));
  else {
    AssignCond always;
    always.tag = ALWAYS;
    Var v = freshVar();
    varAssign(seq, always, v, e);
    genStartDMALoad(seq, srcReg(e->var));
  }
}

void startDMAWriteStmt(Seq<Instr>* seq, Expr* e)
{
  if (e->tag == VAR)
    genStartDMAStore(seq, srcReg(e->var));
  else {
    AssignCond always;
    always.tag = ALWAYS;
    Var v = freshVar();
    varAssign(seq, always, v, e);
    genStartDMAStore(seq, srcReg(e->var));
  }
}

// ============================================================================
// Semaphores
// ============================================================================

void semaphore(Seq<Instr>* seq, StmtTag tag, int semaId)
{
  Instr instr;
  instr.tag = tag == SEMA_INC ? SINC : SDEC;
  instr.semaId = semaId;
  seq->append(instr);
}

// ============================================================================
// Host IRQ
// ============================================================================

void sendIRQToHost(Seq<Instr>* seq)
{
  Instr instr;
  instr.tag = IRQ;
  seq->append(instr);
}


void setupVPMWriteStmt(Seq<Instr>* seq, Expr* e, int hor, int stride)
{
  if (e->tag == INT_LIT)
    genSetupVPMStore(seq, e->intLit, hor, stride);
  else if (e->tag == VAR)
    genSetupVPMStore(seq, srcReg(e->var), hor, stride);
  else {
    AssignCond always;
    always.tag = ALWAYS;
    Var v = freshVar();
    varAssign(seq, always, v, e);
    genSetupVPMStore(seq, srcReg(v), hor, stride);
  }
}


// ============================================================================
// Store request
// ============================================================================

// A 'store' operation of data to addr is almost the same as
// *addr = data.  The difference is that a 'store' waits until
// outstanding DMAs have completed before performing a write rather
// than after a write.  This enables other operations to happen in
// parallel with the write.

void storeRequest(Seq<Instr>* seq, Expr* data, Expr* addr) {
	using namespace QPULib::Target::instr;

  if (data->tag != VAR || addr->tag != VAR) {
    data = putInVar(seq, data);
    addr = putInVar(seq, addr);
  }

  // Setup VPM
  Reg addrReg = freshReg();
  *seq << genLI(addrReg, 16)
       << add(addrReg, addrReg, QPU_ID);
  genSetupVPMStore(seq, addrReg, 0, 1);
  // Store address
  Reg storeAddr = freshReg();
  *seq << genLI(storeAddr, 256)
       << add(storeAddr, storeAddr, QPU_ID);
  // Wait for any outstanding store to complete
  genWaitDMAStore(seq);
  // Setup DMA
  genSetWriteStride(seq, 0);
  genSetupDMAStore(seq, 16, 1, 1, storeAddr);
  // Put to VPM
  Reg dataReg(SPECIAL, SPECIAL_VPM_WRITE);
  seq->append(genLShift(dataReg, srcReg(data->var), 0));
  // Start DMA
  genStartDMAStore(seq, srcReg(addr->var));
}

}  // anon namespace

namespace vc4 {

/**
 * @return true if statement handled, false otherwise
 */
bool stmt(Seq<Instr>* seq, Stmt* s) {

  // ---------------------------------------------
  // Case: store(e0, e1) where e1 and e2 are exprs
  // ---------------------------------------------
  if (s->tag == STORE_REQUEST) {
		storeRequest(seq, s->storeReq.data, s->storeReq.addr);
    return true;
  }

  // --------------------------------------------------------------
  // Case: setReadStride(e) or setWriteStride(e) where e is an expr
  // --------------------------------------------------------------
  if (s->tag == SET_READ_STRIDE || s->tag == SET_WRITE_STRIDE) {
    setStrideStmt(seq, s->tag, s->stride);
    return true;
  }

  // ---------------------------------------------------------------
  // Case: semaInc(n) or semaDec(n) where n is an int (semaphore id)
  // ---------------------------------------------------------------
  if (s->tag == SEMA_INC || s->tag == SEMA_DEC) {
    semaphore(seq, s->tag, s->semaId);
    return true;
  }

  // ---------------
  // Case: hostIRQ()
  // ---------------
  if (s->tag == SEND_IRQ_TO_HOST) {
    sendIRQToHost(seq);
    return true;
  }

  // ----------------------------------------
  // Case: vpmSetupRead(dir, n, addr, stride)
  // ----------------------------------------
  if (s->tag == SETUP_VPM_READ) {
    setupVPMReadStmt(seq,
      s->setupVPMRead.numVecs,
      s->setupVPMRead.addr,
      s->setupVPMRead.hor,
      s->setupVPMRead.stride);
    return true;
  }

  // --------------------------------------
  // Case: vpmSetupWrite(dir, addr, stride)
  // --------------------------------------
  if (s->tag == SETUP_VPM_WRITE) {
    setupVPMWriteStmt(seq,
      s->setupVPMWrite.addr,
      s->setupVPMWrite.hor,
      s->setupVPMWrite.stride);
    return true;
  }

  // ------------------------------------------------------
  // Case: dmaSetupRead(dir, numRows, addr, rowLen, vpitch)
  // ------------------------------------------------------
  if (s->tag == SETUP_DMA_READ) {
    setupDMAReadStmt(seq,
      s->setupDMARead.numRows,
      s->setupDMARead.rowLen,
      s->setupDMARead.hor,
      s->setupDMARead.vpmAddr,
      s->setupDMARead.vpitch);
    return true;
  }

  // -----------------------------------------------
  // Case: dmaSetupWrite(dir, numRows, addr, rowLen)
  // -----------------------------------------------
  if (s->tag == SETUP_DMA_WRITE) {
    setupDMAWriteStmt(seq,
      s->setupDMAWrite.numRows,
      s->setupDMAWrite.rowLen,
      s->setupDMAWrite.hor,
      s->setupDMAWrite.vpmAddr);
    return true;
  }

  // -------------------
  // Case: dmaReadWait()
  // -------------------
  if (s->tag == DMA_READ_WAIT) {
    genWaitDMALoad(seq);
    return true;
  }

  // --------------------
  // Case: dmaWriteWait()
  // --------------------
  if (s->tag == DMA_WRITE_WAIT) {
    genWaitDMAStore(seq);
    return true;
  }

  // ------------------------
  // Case: dmaStartRead(addr)
  // ------------------------
  if (s->tag == DMA_START_READ) {
    startDMAReadStmt(seq, s->startDMARead);
    return true;
  }

  // -------------------------
  // Case: dmaStartWrite(addr)
  // -------------------------
  if (s->tag == DMA_START_WRITE) {
    startDMAWriteStmt(seq, s->startDMAWrite);
    return true;
  }


	return false;
}

}  // namespace vc4
}  // namespace QPULib
