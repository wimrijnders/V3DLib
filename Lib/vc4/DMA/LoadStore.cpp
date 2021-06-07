#include "LoadStore.h"
#include "Support/debug.h" 
#include "Source/Translate.h"
#include "Target/instr/Mnemonics.h"
#include "Helpers.h"

namespace V3DLib {
namespace DMA {
using namespace V3DLib::Target::instr;

namespace {

/**
 * Obtain a register for a fresh variable
 */
Reg freshReg() {
  return Reg(REG_A, VarGen::fresh().id());
}


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

Instr::List genSetupVPMLoad(int n, int addr, int hor, int stride) {
  Instr::List ret;
  assert(addr < 256);

  int setup = vpmSetupReadCode(n, hor, stride) | (addr & 0xff);
  Instr instr;
  instr.tag = VPM_STALL;

  ret << li(RD_SETUP, setup)
      << instr;

  return ret;
}


Instr::List genSetupVPMLoad(int n, Reg addr, int hor, int stride) {
  Instr::List ret;
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


Instr::List genSetupVPMStore(Reg addr, int hor, int stride) {
  Instr::List ret;
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


Instr::List genSetupDMALoad(int numRows, int rowLen, int hor, int vpitch, Reg vpmAddr) {
  Instr::List ret;

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


Instr::List genSetupDMAStore(int numRows, int rowLen, int hor, Reg vpmAddr) {
  int setup = dmaSetupStoreCode(numRows, rowLen, hor);

  Reg tmp0 = freshReg();
  Reg tmp1 = freshReg();

  Instr::List ret;

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


Instr::List genSetReadPitch(Reg pitch) {
  Instr::List ret;
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


Instr::List genSetWriteStride(Reg stride) {
  Reg tmp = freshReg();

  Instr::List ret;
  ret << li(tmp, 0xc0000000)
      << bor(WR_SETUP, tmp, stride);

  return ret;
}


// ============================================================================
// Set-stride statements
// ============================================================================

Instr::List setStrideStmt(bool is_read, Expr::Ptr e) {
  Instr::List ret;

  if (e->tag() == Expr::INT_LIT) {
    if (is_read)
      ret << genSetReadPitch(e->intLit);
    else
      ret << genSetWriteStride(e->intLit);
  } else if (e->tag() == Expr::VAR) {
    Reg reg = srcReg(e->var());

    if (is_read)
      ret << genSetReadPitch(reg);
    else
      ret << genSetWriteStride(reg);
  } else {
    Var v = VarGen::fresh();
    ret << varAssign(v, e);
    if (is_read)
      ret << genSetReadPitch(srcReg(v));
    else
      ret << genSetWriteStride(srcReg(v));
  }

  return ret;
}


Instr::List startDMAReadStmt(Expr::Ptr e) {
  Instr::List ret;

  if (e->tag() != Expr::VAR) {
    Var v = VarGen::fresh();
    ret << varAssign(v, e);
  }

  ret << genStartDMALoad(srcReg(e->var()));
  return ret;
}


Instr::List startDMAWriteStmt(Expr::Ptr e) {
  Instr::List ret;

  if (e->tag() != Expr::VAR) {
    Var v = VarGen::fresh();
    ret << varAssign(v, e);
  }

  ret << genStartDMAStore(srcReg(e->var()));
  return ret;
}


// ============================================================================
// Semaphores
// ============================================================================

Instr semaphore(bool is_inc, int semaId) {
  Instr instr;
  instr.tag = (is_inc)? SINC : SDEC;
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

}  // anon namespace


// ============================================================================
// VPM setup statements
// ============================================================================

Instr::List Stmt::setupVPMRead() {
  Instr::List ret;

  int n       = m_setupVPMRead.numVecs;
  Expr::Ptr e = address_internal();
  int hor     = m_setupVPMRead.hor;
  int stride  = m_setupVPMRead.stride;

  if (e->tag() == Expr::INT_LIT)
    ret << genSetupVPMLoad(n, e->intLit, hor, stride);
  else if (e->tag() == Expr::VAR)
    ret << genSetupVPMLoad(n, srcReg(e->var()), hor, stride);
  else {
    Var v = VarGen::fresh();
    ret << varAssign(v, e)
        << genSetupVPMLoad(n, srcReg(v), hor, stride);
  }

  return ret;
}


// ============================================================================
// DMA statements
// ============================================================================

Instr::List Stmt::setupDMARead() {
  int numRows = m_setupDMARead.numRows;
  int rowLen  = m_setupDMARead.rowLen;
  int hor     = m_setupDMARead.hor;
  Expr::Ptr e = address_internal();
  int vpitch  = m_setupDMARead.vpitch;

  Instr::List ret;

  if (e->tag() == Expr::INT_LIT)
    ret << genSetupDMALoad(numRows, rowLen, hor, vpitch, e->intLit);
  else if (e->tag() == Expr::VAR)
    ret << genSetupDMALoad(numRows, rowLen, hor, vpitch, srcReg(e->var()));
  else {
    Var v = VarGen::fresh();

    ret << varAssign(v, e)
        << genSetupDMALoad(numRows, rowLen, hor, vpitch, srcReg(v));
  }

  return ret;
}


Instr::List Stmt::setupDMAWrite() {
  int numRows = m_setupDMAWrite.numRows;
  int rowLen  = m_setupDMAWrite.rowLen;
  int hor     = m_setupDMAWrite.hor;
  Expr::Ptr e = address_internal();

  Instr::List ret;

  if (e->tag() == Expr::INT_LIT) {
    ret << genSetupDMAStore(numRows, rowLen, hor, e->intLit);
  } else if (e->tag() == Expr::VAR) {
    ret << genSetupDMAStore(numRows, rowLen, hor, srcReg(e->var()));
  } else {
    Var v = VarGen::fresh();

    ret << varAssign(v, e)
        << genSetupDMAStore(numRows, rowLen, hor, srcReg(v));
  }

  return ret;
}


Instr::List Stmt::setupVPMWrite() {
  Instr::List ret;

  Expr::Ptr e = address_internal();
  int hor     = m_setupVPMWrite.hor;
  int stride  = m_setupVPMWrite.stride;

  if (e->tag() == Expr::INT_LIT)
    ret << genSetupVPMStore(e->intLit, hor, stride);
  else if (e->tag() == Expr::VAR)
    ret << genSetupVPMStore(srcReg(e->var()), hor, stride);
  else {
    Var v = VarGen::fresh();
    ret << varAssign(v, e)
        << genSetupVPMStore(srcReg(v), hor, stride);
  }

  return ret;
}


/**
 * Load vector `dst` from address in main memory as specified by `e`
 */
Instr::List loadRequest(Var &dst, Expr &e) {
  using namespace V3DLib::Target::instr;

  Reg reg = srcReg(e.deref_ptr()->var());
  Instr::List ret;
  
  ret << genSetReadPitch(4).comment("Start DMA load var")                          // Setup DMA
      << genSetupDMALoad(16, 1, 1, 1, QPU_ID)
      << genStartDMALoad(reg)                                                      // Start DMA load
      << genWaitDMALoad(false)                                                     // Wait for DMA
      << genSetupVPMLoad(1, QPU_ID, 0, 1)                                          // Setup VPM
      << shl(dstReg(dst), Target::instr::VPM_READ, 0).comment("End DMA load var"); // Get from VPM

  return ret;
}


/**
 * Save vector `src` to destination addres in main memory
 */
Instr::List storeRequest(Var dst_addr, Var src) {
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
      << shl(Target::instr::VPM_WRITE, srcReg(src), 0)                         // Put to VPM
      << genStartDMAStore(srcReg(dst_addr)).comment("End DMA store request");  // Start DMA

  return ret;
}


/**
 * @return true if statement handled, false otherwise
 */
bool translate_stmt(Instr::List &seq, int in_tag, Stmt &s) {
  auto tag = to_tag(in_tag);
  bool ret = true;

  switch (tag) {
    case V3DLib::Stmt::SET_READ_STRIDE:  seq << setStrideStmt(true,  s.stride_internal()); break;
    case V3DLib::Stmt::SET_WRITE_STRIDE: seq << setStrideStmt(false, s.stride_internal()); break;
    case V3DLib::Stmt::SEMA_INC:         seq << semaphore(true,  s.semaId());              break;
    case V3DLib::Stmt::SEMA_DEC:         seq << semaphore(false, s.semaId());              break;
    case V3DLib::Stmt::SEND_IRQ_TO_HOST: seq << sendIRQToHost();                       break;
    case V3DLib::Stmt::SETUP_VPM_READ:   seq << s.setupVPMRead();                      break;
    case V3DLib::Stmt::SETUP_VPM_WRITE:  seq << s.setupVPMWrite();                     break;
    case V3DLib::Stmt::SETUP_DMA_READ:   seq << s.setupDMARead();                      break;
    case V3DLib::Stmt::SETUP_DMA_WRITE:  seq << s.setupDMAWrite();                     break;
    case V3DLib::Stmt::DMA_READ_WAIT:    seq << genWaitDMALoad();                      break;
    case V3DLib::Stmt::DMA_WRITE_WAIT:   seq << genWaitDMAStore();                     break;
    case V3DLib::Stmt::DMA_START_READ:   seq << startDMAReadStmt(s.address_internal());  break;
    case V3DLib::Stmt::DMA_START_WRITE:  seq << startDMAWriteStmt(s.address_internal()); break;

    default:
      ret = false;
      break;
  }

  return ret;
}

}  // namespace DMA
}  // namespace V3DLib
