#include "LoadStore.h"
#include <assert.h>
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace QPULib {
	using namespace QPULib::Target::instr;

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

void genSetupVPMLoad(Seq<Instr>* instrs, int n, int addr, int hor, int stride) {
  assert(addr < 256);

  int setup = vpmSetupReadCode(n, hor, stride) | (addr & 0xff);
  Instr instr;
  instr.tag = VPM_STALL;

  *instrs << li(RD_SETUP, setup)
          << instr;
}

void genSetupVPMLoad(Seq<Instr>* instrs, int n, Reg addr, int hor, int stride) {
  Reg tmp = freshReg();
  int setup = vpmSetupReadCode(n, hor, stride);

  Instr instr;
  instr.tag = VPM_STALL;

  *instrs << li(tmp, setup)
          << bor(RD_SETUP, addr, tmp)
          << instr;
}

// Generate instructions to setup VPM store.

void genSetupVPMStore(Seq<Instr>* instrs, int addr, int hor, int stride) {
  assert(addr < 256);

  int setup = vpmSetupWriteCode(hor, stride) | (addr & 0xff);
  *instrs << li(WR_SETUP, setup);
}


void genSetupVPMStore(Seq<Instr>* instrs, Reg addr, int hor, int stride) {
  Reg tmp = freshReg();
  int setup = vpmSetupWriteCode(hor, stride);

  *instrs << li(tmp, setup)
          << bor(WR_SETUP, addr, tmp);
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

void genSetupDMALoad( Seq<Instr>* instrs, int numRows, int rowLen, int hor, int vpitch, int vpmAddr) {
  assert(vpmAddr < 2048);

  int setup = dmaSetupLoadCode(numRows, rowLen, hor, vpitch);
  setup |= vpmAddr;

  *instrs << li(RD_SETUP, setup);
}

void genSetupDMALoad( Seq<Instr>* instrs, int numRows, int rowLen, int hor, int vpitch, Reg vpmAddr) {
  int setup = dmaSetupLoadCode(numRows, rowLen, hor, vpitch);
  Reg tmp = freshReg();

  *instrs << li(tmp, setup)
	        << bor(RD_SETUP, vpmAddr, tmp);
}


void genStartDMALoad(Seq<Instr>* instrs, Reg memAddr) {
  *instrs << mov(DMA_LD_ADDR, memAddr);
}


void genWaitDMALoad(Seq<Instr>* instrs) {
	*instrs << mov(None, DMA_LD_WAIT, AssignCond::never);
/*
  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = false;
  instr.ALU.cond.tag          = NEVER;
  instr.ALU.op                = A_BOR;

  instr.ALU.dest.tag          = NONE;

  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg.tag      = SPECIAL;
  instr.ALU.srcA.reg.regId    = SPECIAL_DMA_LD_WAIT;
  instr.ALU.srcB.tag          = REG;
  instr.ALU.srcB.reg          = instr.ALU.srcA.reg;

  *instrs << instr;
*/
}

// Generate instructions to do DMA store.

void genSetupDMAStore( Seq<Instr>* instrs, int numRows, int rowLen, int hor, int vpmAddr) {
  assert(vpmAddr < 2048);

  int setup = dmaSetupStoreCode(numRows, rowLen, hor);
  setup |= vpmAddr << 3;

  *instrs << li(WR_SETUP, setup);
}


void genSetupDMAStore( Seq<Instr>* instrs, int numRows, int rowLen, int hor, Reg vpmAddr) {
  int setup = dmaSetupStoreCode(numRows, rowLen, hor);

  Reg tmp0 = freshReg();
  Reg tmp1 = freshReg();

  *instrs << li(tmp0, setup)
          << shl(tmp1, vpmAddr, 3)
          << bor(WR_SETUP, tmp0, tmp1);
}


void genStartDMAStore(Seq<Instr>* instrs, Reg memAddr) {
  *instrs << mov(DMA_ST_ADDR, memAddr);
}


void genWaitDMAStore(Seq<Instr>* instrs) {
	*instrs << mov(None, DMA_ST_WAIT, AssignCond::NEVER);
}

// =============================================================================
// DMA stride setup
// =============================================================================

// Generate instructions to set the DMA read pitch.

void genSetReadPitch(Seq<Instr>* instrs, int pitch) {
  assert(pitch < 8192);

  int setup = 0x90000000 | pitch;
  *instrs << li(RD_SETUP, setup);
}

void genSetReadPitch(Seq<Instr>* instrs, Reg pitch) {
  Reg tmp = freshReg();

  *instrs << li(tmp, 0x90000000)
          << bor(RD_SETUP, tmp, pitch);
}

// Generate instructions to set the DMA write stride.

void genSetWriteStride(Seq<Instr>* instrs, int stride) {
  assert(stride < 8192);

  int setup = 0xc0000000 | stride;
  *instrs << li(WR_SETUP, setup);
}

void genSetWriteStride(Seq<Instr>* instrs, Reg stride) {
  Reg tmp = freshReg();

  *instrs << li(tmp, 0xc0000000)
          << bor(WR_SETUP, tmp, stride);
}
}  // namespace QPULib
