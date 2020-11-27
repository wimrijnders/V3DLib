#include "Translate.h"
#include "Source/Translate.h"
#include "LoadStore.h"

namespace V3DLib {

namespace {

// ============================================================================
// Set-stride statements
// ============================================================================

void setStrideStmt(Seq<Instr>* seq, StmtTag tag, Expr* e) {
  if (e->tag() == INT_LIT) {
    if (tag == SET_READ_STRIDE)
      *seq << genSetReadPitch(e->intLit);
    else
      *seq << genSetWriteStride(e->intLit);
  }
  else if (e->tag() == VAR) {
    if (tag == SET_READ_STRIDE)
      genSetReadPitch(seq, srcReg(e->var));
    else
      *seq << genSetWriteStride(srcReg(e->var));
  }
  else {
    Var v = freshVar();
    varAssign(seq, v, e);
    if (tag == SET_READ_STRIDE)
      genSetReadPitch(seq, srcReg(v));
    else
      *seq << genSetWriteStride(srcReg(v));
  }
}

// ============================================================================
// VPM setup statements
// ============================================================================

void setupVPMReadStmt(Seq<Instr>* seq, int n, Expr* e, int hor, int stride) {
  if (e->tag() == INT_LIT)
    *seq << genSetupVPMLoad(n, e->intLit, hor, stride);
  else if (e->tag() == VAR)
    *seq << genSetupVPMLoad(n, srcReg(e->var), hor, stride);
  else {
    Var v = freshVar();
    varAssign(seq, v, e);
    *seq << genSetupVPMLoad(n, srcReg(v), hor, stride);
  }
}

// ============================================================================
// DMA statements
// ============================================================================

void setupDMAReadStmt(Seq<Instr>* seq, int numRows, int rowLen, int hor, Expr* e, int vpitch) {
  if (e->tag() == INT_LIT)
    *seq << genSetupDMALoad(numRows, rowLen, hor, vpitch, e->intLit);
  else if (e->tag() == VAR)
    *seq << genSetupDMALoad(numRows, rowLen, hor, vpitch, srcReg(e->var));
  else {
    Var v = freshVar();
    varAssign(seq, v, e);
    *seq << genSetupDMALoad(numRows, rowLen, hor, vpitch, srcReg(v));
  }
}

void setupDMAWriteStmt(Seq<Instr>* seq, int numRows, int rowLen, int hor, Expr* e) {
  if (e->tag() == INT_LIT)
    *seq << genSetupDMAStore(numRows, rowLen, hor, e->intLit);
  else if (e->tag() == VAR)
    *seq << genSetupDMAStore(numRows, rowLen, hor, srcReg(e->var));
  else {
    Var v = freshVar();
    varAssign(seq, v, e);
    *seq << genSetupDMAStore(numRows, rowLen, hor, srcReg(v));
  }
}


void startDMAReadStmt(Seq<Instr>* seq, Expr* e) {
  if (e->tag() == VAR)
    *seq << genStartDMALoad(srcReg(e->var));
  else {
    Var v = freshVar();
    varAssign(seq, v, e);
    *seq << genStartDMALoad(srcReg(e->var));
  }
}


Instr startDMAWriteStmt(Seq<Instr>* seq, Expr* e) {
  if (e->tag() == VAR)
    return genStartDMAStore(srcReg(e->var));
  else {
    Var v = freshVar();
    varAssign(seq, v, e);
    return genStartDMAStore(srcReg(e->var));
  }
}


// ============================================================================
// Semaphores
// ============================================================================

Instr semaphore(StmtTag tag, int semaId) {
  Instr instr;
  instr.tag = (tag == SEMA_INC)? SINC : SDEC;
  instr.semaId = semaId;

	return instr;
}


// ============================================================================
// Host IRQ
// ============================================================================

Instr sendIRQToHost() {
  Instr instr;
  instr.tag = IRQ;
  return instr;
}


void setupVPMWriteStmt(Seq<Instr>* seq, Expr* e, int hor, int stride) {
  if (e->tag() == INT_LIT)
    *seq << genSetupVPMStore(e->intLit, hor, stride);
  else if (e->tag() == VAR)
    *seq << genSetupVPMStore(srcReg(e->var), hor, stride);
  else {
    Var v = freshVar();
    varAssign(seq, v, e);
    *seq << genSetupVPMStore(srcReg(v), hor, stride);
  }
}


// ============================================================================
// Store request operation
// ============================================================================

// A 'store' operation of data to addr is almost the same as
// *addr = data.  The difference is that a 'store' waits until
// outstanding DMAs have completed before performing a write rather
// than after a write.  This enables other operations to happen in
// parallel with the write.

void storeRequestOperation(Seq<Instr> &seq, Expr *data, Expr *addr) {
	using namespace V3DLib::Target::instr;

  if (data->tag() != VAR || addr->tag() != VAR) {
    data = putInVar(&seq, data);
    addr = putInVar(&seq, addr);
  }

	seq << vc4::StoreRequest(addr->var, data->var, true);
}

}  // anon namespace


namespace vc4 {

/**
 * @return true if statement handled, false otherwise
 */
bool stmt(Seq<Instr> *seq, Stmt *s) {

  // ---------------------------------------------
  // Case: store(e0, e1) where e1 and e2 are exprs
  // ---------------------------------------------
  if (s->tag == STORE_REQUEST) {
		storeRequestOperation(*seq, s->storeReq.data, s->storeReq.addr);
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
    *seq << semaphore(s->tag, s->semaId);
    return true;
  }

  // ---------------
  // Case: hostIRQ()
  // ---------------
  if (s->tag == SEND_IRQ_TO_HOST) {
    *seq << sendIRQToHost();
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
    *seq << genWaitDMALoad();
    return true;
  }

  // --------------------
  // Case: dmaWriteWait()
  // --------------------
  if (s->tag == DMA_WRITE_WAIT) {
    *seq << genWaitDMAStore();
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


// ============================================================================
// Store request
// ============================================================================

Seq<Instr> StoreRequest(Var addr_var, Var data_var,  bool wait) {
	using namespace V3DLib::Target::instr;

	Reg addr      = freshReg();
	Reg storeAddr = freshReg();

	Seq<Instr> ret;

	ret << li(addr, 16)                       // Setup VPM
	    << add(addr, addr, QPU_ID)
	    << genSetupVPMStore(addr, 0, 1)
	    << li(storeAddr, 256)                 // Store address
	    << add(storeAddr, storeAddr, QPU_ID);

	if (wait) {
		ret << genWaitDMAStore();                             // Wait for any outstanding store to complete
	}

	// Setup DMA
	ret << genSetWriteStride(0)
	    << genSetupDMAStore(16, 1, 1, storeAddr)
	    << shl(Target::instr::VPM_WRITE, srcReg(data_var), 0)  // Put to VPM
	    << genStartDMAStore(srcReg(addr_var));                 // Start DMA

	ret.front().comment("Start DMA store request");
	ret.back().comment("End DMA store request");

	return ret;
}

}  // namespace vc4
}  // namespace V3DLib
