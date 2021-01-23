#include "Translate.h"
#include "Source/Stmt.h"
#include "Source/Translate.h"
#include "LoadStore.h"

namespace V3DLib {

namespace {

// ============================================================================
// Set-stride statements
// ============================================================================

Seq<Instr> setStrideStmt(StmtTag tag, Expr::Ptr e) {
  Seq<Instr> ret;

  if (e->tag() == Expr::INT_LIT) {
    if (tag == SET_READ_STRIDE)
      ret << genSetReadPitch(e->intLit);
    else
      ret << genSetWriteStride(e->intLit);
  } else if (e->tag() == Expr::VAR) {
    Reg reg = srcReg(e->var());

    if (tag == SET_READ_STRIDE)
      ret << genSetReadPitch(reg);
    else
      ret << genSetWriteStride(reg);
  } else {
    Var v = freshVar();
    ret << varAssign(v, e);
    if (tag == SET_READ_STRIDE)
      ret << genSetReadPitch(srcReg(v));
    else
      ret << genSetWriteStride(srcReg(v));
  }

  return ret;
}

// ============================================================================
// VPM setup statements
// ============================================================================

Seq<Instr> setupVPMReadStmt(Stmt::Ptr s) {
  Seq<Instr> ret;

  int n       = s->setupVPMRead.numVecs;
  Expr::Ptr e = s->address();
  int hor     = s->setupVPMRead.hor;
  int stride  = s->setupVPMRead.stride;

  if (e->tag() == Expr::INT_LIT)
    ret << genSetupVPMLoad(n, e->intLit, hor, stride);
  else if (e->tag() == Expr::VAR)
    ret << genSetupVPMLoad(n, srcReg(e->var()), hor, stride);
  else {
    Var v = freshVar();
    ret << varAssign(v, e)
        << genSetupVPMLoad(n, srcReg(v), hor, stride);
  }

  return ret;
}


// ============================================================================
// DMA statements
// ============================================================================

Seq<Instr> setupDMAReadStmt(Stmt::Ptr s) {
  int numRows = s->setupDMARead.numRows;
  int rowLen  = s->setupDMARead.rowLen;
  int hor     = s->setupDMARead.hor;
  Expr::Ptr e = s->address();
  int vpitch  = s->setupDMARead.vpitch;

  Seq<Instr> ret;

  if (e->tag() == Expr::INT_LIT)
    ret << genSetupDMALoad(numRows, rowLen, hor, vpitch, e->intLit);
  else if (e->tag() == Expr::VAR)
    ret << genSetupDMALoad(numRows, rowLen, hor, vpitch, srcReg(e->var()));
  else {
    Var v = freshVar();

    ret << varAssign(v, e)
        << genSetupDMALoad(numRows, rowLen, hor, vpitch, srcReg(v));
  }

  return ret;
}


Seq<Instr> setupDMAWriteStmt(Stmt::Ptr s) {
  int numRows = s->setupDMAWrite.numRows;
  int rowLen  = s->setupDMAWrite.rowLen;
  int hor     = s->setupDMAWrite.hor;
  Expr::Ptr e = s->address();

  Seq<Instr> ret;

  if (e->tag() == Expr::INT_LIT) {
    ret << genSetupDMAStore(numRows, rowLen, hor, e->intLit);
  } else if (e->tag() == Expr::VAR) {
    ret << genSetupDMAStore(numRows, rowLen, hor, srcReg(e->var()));
  } else {
    Var v = freshVar();

    ret << varAssign(v, e)
        << genSetupDMAStore(numRows, rowLen, hor, srcReg(v));
  }

  return ret;
}


Seq<Instr> startDMAReadStmt(Expr::Ptr e) {
  Seq<Instr> ret;

  if (e->tag() != Expr::VAR) {
    Var v = freshVar();
    ret << varAssign(v, e);
  }

  ret << genStartDMALoad(srcReg(e->var()));
  return ret;
}


Seq<Instr> startDMAWriteStmt(Expr::Ptr e) {
  Seq<Instr> ret;

  if (e->tag() != Expr::VAR) {
    Var v = freshVar();
    ret << varAssign(v, e);
  }

  ret << genStartDMAStore(srcReg(e->var()));
  return ret;
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


Seq<Instr> setupVPMWriteStmt(Stmt::Ptr s) {
  Seq<Instr> ret;

  Expr::Ptr e = s->address();
  int hor     = s->setupVPMWrite.hor;
  int stride  = s->setupVPMWrite.stride;

  if (e->tag() == Expr::INT_LIT)
    ret << genSetupVPMStore(e->intLit, hor, stride);
  else if (e->tag() == Expr::VAR)
    ret << genSetupVPMStore(srcReg(e->var()), hor, stride);
  else {
    Var v = freshVar();
    ret << varAssign(v, e)
        << genSetupVPMStore(srcReg(v), hor, stride);
  }

  return ret;
}


// ============================================================================
// Store request operation
// ============================================================================

// A 'store' operation of data to addr is almost the same as
// *addr = data.  The difference is that a 'store' waits until
// outstanding DMAs have completed before performing a write rather
// than after a write.  This enables other operations to happen in
// parallel with the write.

Seq<Instr> storeRequestOperation(Stmt::Ptr s) {
  Expr::Ptr data = s->storeReq_data();
  Expr::Ptr addr = s->storeReq_addr();

  Seq<Instr> ret;

  if (data->tag() != Expr::VAR || addr->tag() != Expr::VAR) {
    data = putInVar(&ret, data);
    addr = putInVar(&ret, addr);
  }

  ret << vc4::StoreRequest(addr->var(), data->var(), true);
  return ret;
}

}  // anon namespace


namespace vc4 {

/**
 * @return true if statement handled, false otherwise
 */
bool translate_stmt(Seq<Instr> &seq, Stmt::Ptr s) {

  switch (s->tag) {
    case STORE_REQUEST:    seq << storeRequestOperation(s);              return true;
    case SET_READ_STRIDE:
    case SET_WRITE_STRIDE: seq << setStrideStmt(s->tag, s->stride());    return true;
    case SEMA_INC:
    case SEMA_DEC:         seq << semaphore(s->tag, s->semaId);          return true;
    case SEND_IRQ_TO_HOST: seq << sendIRQToHost();                       return true;
    case SETUP_VPM_READ:   seq << setupVPMReadStmt(s);                   return true;
    case SETUP_VPM_WRITE:  seq << setupVPMWriteStmt(s);                  return true;
    case SETUP_DMA_READ:   seq << setupDMAReadStmt(s);                   return true;
    case SETUP_DMA_WRITE:  seq << setupDMAWriteStmt(s);                  return true;
    case DMA_READ_WAIT:    seq << genWaitDMALoad();                      return true;
    case DMA_WRITE_WAIT:   seq << genWaitDMAStore();                     return true;
    case DMA_START_READ:   seq<< startDMAReadStmt(s->address());         return true;
    case DMA_START_WRITE:  seq << startDMAWriteStmt(s->address());       return true;

    default:
      assertq(false, "translate_stmt(): unexpected stmt tag");
      break;
  }

  return false;
}


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
