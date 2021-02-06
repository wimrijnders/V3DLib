#include "LoadStore.h"
#include "Support/debug.h" 
#include "Source/Translate.h"
#include "Target/Syntax.h"

namespace V3DLib {
namespace DMA {
using namespace V3DLib::Target::instr;

namespace {

// =============================================================================
// VPM setup
// =============================================================================

static int vpmSetupReadCode(int n, int hor, int stride) {
  assert(n >= 1 && n <= 16); // A max of 16 vectors can be read
  assert(stride >= 1 && stride <= 64); // Valid stride
  assert(hor == 0 || hor == 1); // Horizontal or vertical

  // Max values encoded as 0
  if (n == 16) n = 0;
  if (stride == 64) stride = 0;

  // Setup code
  int code = n << 20;
  code |= stride << 12;
  code |= hor << 11;
  code |= 2 << 8;

  return code;
}

static int vpmSetupWriteCode(int hor, int stride) {
  assert(stride >= 1 && stride <= 64); // Valid stride
  assert(hor == 0 || hor == 1); // Horizontal or vertical

  // Max values encoded as 0
  if (stride == 64) stride = 0;
  
  // Setup code
  int code = stride << 12;
  code |= hor << 11;
  code |= 2 << 8;
  
  return code;
}

// Generate instructions to setup VPM load.

Seq<Instr> genSetupVPMLoad(int n, int addr, int hor, int stride) {
  Seq<Instr> ret;
  assert(addr < 256);

  int setup = vpmSetupReadCode(n, hor, stride) | (addr & 0xff);
  Instr instr;
  instr.tag = VPM_STALL;

  ret << li(RD_SETUP, setup)
      << instr;

  return ret;
}


Seq<Instr> genSetupVPMLoad(int n, Reg addr, int hor, int stride) {
  Seq<Instr> ret;
  Reg tmp = freshReg();
  int setup = vpmSetupReadCode(n, hor, stride);

  Instr instr;
  instr.tag = VPM_STALL;

  ret << li(tmp, setup)
      << bor(RD_SETUP, addr, tmp)
      << instr;

  return ret;
}

// Generate instructions to setup VPM store.

Instr genSetupVPMStore(int addr, int hor, int stride) {
  assert(addr < 256);
  int setup = vpmSetupWriteCode(hor, stride) | (addr & 0xff);

  return li(WR_SETUP, setup);
}


Seq<Instr> genSetupVPMStore(Reg addr, int hor, int stride) {
  Seq<Instr> ret;
  Reg tmp = freshReg();
  int setup = vpmSetupWriteCode(hor, stride);

  ret << li(tmp, setup)
      << bor(WR_SETUP, addr, tmp);
  return ret;
}

// =============================================================================
// DMA setup
// =============================================================================

// (rowLen in bytes)
static int dmaSetupStoreCode(int numRows, int rowLen, int hor)
{
  assert(numRows > 0 && numRows <= 128);
  assert(rowLen > 0 && rowLen <= 128);
  if (numRows == 128) numRows = 0;
  if (rowLen == 128) rowLen = 0;

  int setup = 0x80000000;
  setup |= numRows << 23;
  setup |= rowLen << 16;
  setup |= hor << 14;
  return setup;
}

// (rowLen in 32-bit words)
static int dmaSetupLoadCode(int numRows, int rowLen, int hor, int vpitch)
{
  assert(numRows > 0 && numRows <= 16);
  assert(rowLen > 0 && rowLen <= 16);
  assert(vpitch > 0 && vpitch <= 16);
  if (numRows == 16) numRows = 0;
  if (rowLen == 16) rowLen = 0;
  if (vpitch == 16) vpitch = 0;

  int setup = 0x80000000;
  setup |= rowLen << 20;
  setup |= numRows << 16;
  setup |= vpitch << 12;
  setup |= (hor == 0 ? 1 : 0) << 11;
  return setup;
}

// Generate instructions to setup DMA load.

Instr genSetupDMALoad(int numRows, int rowLen, int hor, int vpitch, int vpmAddr) {
  assert(vpmAddr < 2048);

  int setup = dmaSetupLoadCode(numRows, rowLen, hor, vpitch);
  setup |= vpmAddr;

  return li(RD_SETUP, setup);
}


Seq<Instr> genSetupDMALoad(int numRows, int rowLen, int hor, int vpitch, Reg vpmAddr) {
  Seq<Instr> ret;

  int setup = dmaSetupLoadCode(numRows, rowLen, hor, vpitch);
  Reg tmp = freshReg();

  ret << li(tmp, setup)
      << bor(RD_SETUP, vpmAddr, tmp);

  return ret;
}


Instr genStartDMALoad(Reg memAddr) {
  return mov(DMA_LD_ADDR, memAddr);
}


// TODO change var naming
Instr genWaitDMALoad(bool might_be_end = false) {
  Instr instr = mov(None, DMA_LD_WAIT).cond(never);

  if (might_be_end) {
    instr.comment("DMA load wait (likely start of program end)");
  }

  return instr;
}

// Generate instructions to do DMA store.

Instr genSetupDMAStore(int numRows, int rowLen, int hor, int vpmAddr) {
  assert(vpmAddr < 2048);
  int setup = dmaSetupStoreCode(numRows, rowLen, hor);
  setup |= vpmAddr << 3;

  return li(WR_SETUP, setup);
}


Seq<Instr> genSetupDMAStore(int numRows, int rowLen, int hor, Reg vpmAddr) {
  int setup = dmaSetupStoreCode(numRows, rowLen, hor);

  Reg tmp0 = freshReg();
  Reg tmp1 = freshReg();

  Seq<Instr> ret;

  ret << li(tmp0, setup)
      << shl(tmp1, vpmAddr, 3)
      << bor(WR_SETUP, tmp0, tmp1);

  return ret;
}


Instr genStartDMAStore(Reg memAddr) {
  return mov(DMA_ST_ADDR, memAddr);
}


Instr genWaitDMAStore() {
  return mov(None, DMA_ST_WAIT).cond(AssignCond::NEVER);
}


// =============================================================================
// DMA stride setup
// =============================================================================

// Generate instructions to set the DMA read pitch.

Instr genSetReadPitch(int pitch) {
  assert(pitch < 8192);

  int setup = 0x90000000 | pitch;
  return li(RD_SETUP, setup);
}


Seq<Instr> genSetReadPitch(Reg pitch) {
  Seq<Instr> ret;
  Reg tmp = freshReg();

  ret << li(tmp, 0x90000000)
      << bor(RD_SETUP, tmp, pitch);

  return ret;
}


/**
 * Generate instructions to set the DMA write stride.
 */
Instr genSetWriteStride(int stride) {
  assert(stride < 8192);
  int setup = 0xc0000000 | stride;

  return li(WR_SETUP, setup);
}


Seq<Instr> genSetWriteStride(Reg stride) {
  Reg tmp = freshReg();

  Seq<Instr> ret;
  ret << li(tmp, 0xc0000000)
      << bor(WR_SETUP, tmp, stride);

  return ret;
}


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

}  // anon namespace


Instr::List varassign_deref_var(Var &v, Expr &e) {
  using namespace V3DLib::Target::instr;

  Reg reg = srcReg(e.deref_ptr()->var());
  Instr::List ret;
  
  ret << genSetReadPitch(4).comment("Start DMA load var")                        // Setup DMA
      << genSetupDMALoad(16, 1, 1, 1, QPU_ID)
      << genStartDMALoad(reg)                                                    // Start DMA load
      << genWaitDMALoad(false)                                                   // Wait for DMA
      << genSetupVPMLoad(1, QPU_ID, 0, 1)                                        // Setup VPM
      << shl(dstReg(v), Target::instr::VPM_READ, 0).comment("End DMA load var"); // Get from VPM

  return ret;
}


/**
 * @return true if statement handled, false otherwise
 */
bool translate_stmt(Instr::List &seq, Stmt::Ptr s) {
  bool ret = true;

  switch (s->tag) {
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


Instr::List StoreRequest(Var addr_var, Var data_var) {
  using namespace V3DLib::Target::instr;

  Reg addr      = freshReg();
  Reg storeAddr = freshReg();

  Instr::List ret;

  ret << li(addr, 16).comment("Start DMA store request")                       // Setup VPM
      << add(addr, addr, QPU_ID)
      << genSetupVPMStore(addr, 0, 1)
      << li(storeAddr, 256)                                                    // Store address
      << add(storeAddr, storeAddr, QPU_ID)
      << genWaitDMAStore()                                                  // Wait for any previous store to complete

      << genSetWriteStride(0)                                                  // Setup DMA
      << genSetupDMAStore(16, 1, 1, storeAddr)
      << shl(Target::instr::VPM_WRITE, srcReg(data_var), 0)                    // Put to VPM
      << genStartDMAStore(srcReg(addr_var)).comment("End DMA store request");  // Start DMA

  return ret;
}

}  // namespace DMA
}  // namespace V3DLib
