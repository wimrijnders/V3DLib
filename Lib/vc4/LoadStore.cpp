#include "LoadStore.h"
#include <assert.h>
#include "Target/Syntax.h"

namespace V3DLib {
	using namespace V3DLib::Target::instr;

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


Instr genWaitDMALoad(bool might_be_end) {
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

}  // namespace V3DLib
