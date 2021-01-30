#include "Translate.h"
#include "Source/Stmt.h"
#include "Source/Translate.h"
#include "LoadStore.h"

namespace V3DLib {
namespace {

// ============================================================================
// Set-stride statements
// ============================================================================

Seq<Instr> setStrideStmt(Stmt::Tag tag, Expr::Ptr e) {
  Seq<Instr> ret;

  if (e->tag() == Expr::INT_LIT) {
    if (tag == Stmt::SET_READ_STRIDE)
      ret << genSetReadPitch(e->intLit);
    else
      ret << genSetWriteStride(e->intLit);
  } else if (e->tag() == Expr::VAR) {
    Reg reg = srcReg(e->var());

    if (tag == Stmt::SET_READ_STRIDE)
      ret << genSetReadPitch(reg);
    else
      ret << genSetWriteStride(reg);
  } else {
    Var v = freshVar();
    ret << varAssign(v, e);
    if (tag == Stmt::SET_READ_STRIDE)
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

Instr semaphore(Stmt::Tag tag, int semaId) {
  Instr instr;
  instr.tag = (tag == Stmt::SEMA_INC)? SINC : SDEC;
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

Instr::List storeRequestOperation(Stmt::Ptr s) {
  Expr::Ptr data = s->storeReq_data();
  Expr::Ptr addr = s->storeReq_addr();

  Instr::List ret;

  if (data->tag() != Expr::VAR || addr->tag() != Expr::VAR) {
    data = putInVar(&ret, data);
    addr = putInVar(&ret, addr);
  }

  ret << DMA::StoreRequest(addr->var(), data->var(), true);
  return ret;
}

}  // anon namespace


namespace DMA {

/**
 * @return true if statement handled, false otherwise
 */
bool translate_stmt(Instr::List &seq, Stmt::Ptr s) {
  bool ret = true;

  switch (s->tag) {
    case Stmt::STORE_REQUEST:    seq << storeRequestOperation(s);              break;
    case Stmt::SET_READ_STRIDE:
    case Stmt::SET_WRITE_STRIDE: seq << setStrideStmt(s->tag, s->stride());    break;
    case Stmt::SEMA_INC:
    case Stmt::SEMA_DEC:         seq << semaphore(s->tag, s->semaId);          break;
    case Stmt::SEND_IRQ_TO_HOST: seq << sendIRQToHost();                       break;
    case Stmt::SETUP_VPM_READ:   seq << setupVPMReadStmt(s);                   break;
    case Stmt::SETUP_VPM_WRITE:  seq << setupVPMWriteStmt(s);                  break;
    case Stmt::SETUP_DMA_READ:   seq << setupDMAReadStmt(s);                   break;
    case Stmt::SETUP_DMA_WRITE:  seq << setupDMAWriteStmt(s);                  break;
    case Stmt::DMA_READ_WAIT:    seq << genWaitDMALoad();                      break;
    case Stmt::DMA_WRITE_WAIT:   seq << genWaitDMAStore();                     break;
    case Stmt::DMA_START_READ:   seq<< startDMAReadStmt(s->address());         break;
    case Stmt::DMA_START_WRITE:  seq << startDMAWriteStmt(s->address());       break;

    default:
      ret = false;
      break;
  }

  return ret;
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

}  // namespace DMA
}  // namespace V3DLib
