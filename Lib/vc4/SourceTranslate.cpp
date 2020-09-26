#include "SourceTranslate.h"
#include "Support/debug.h"
#include "Source/Translate.h"  // srcReg()
#include "Target/LoadStore.h"
#include "Target/Liveness.h"
#include "Target/Subst.h"
#include "RegAlloc.h"

namespace QPULib {

namespace {

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

}  // anon namespace


namespace vc4 {

bool SourceTranslate::deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr *rhs) {
	using namespace QPULib::Target::instr;
	assert(seq != nullptr);
	assert(rhs != nullptr);

	// Setup VPM
	Reg addr = freshReg();
	*seq << genLI(addr, 16)
	     << add(addr, addr, QPU_ID);
	genSetupVPMStore(seq, addr, 0, 1);
	// Store address
	Reg storeAddr = freshReg();
	*seq << genLI(storeAddr, 256)
	     << add(storeAddr, storeAddr, QPU_ID);
	// Setup DMA
	genSetWriteStride(seq, 0);
	genSetupDMAStore(seq, 16, 1, 1, storeAddr);
	// Put to VPM
	Reg data(SPECIAL, SPECIAL_VPM_WRITE);
	seq->append(genLShift(data, srcReg(rhs->var), 0));
	// Start DMA
	genStartDMAStore(seq, srcReg(lhs.deref.ptr->var));
	// Wait for store to complete
	genWaitDMAStore(seq);

	return true;
}


/**
 * See comment and preamble code in caller: Target/Translate.cpp, line 52
 */
void SourceTranslate::varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) {
	using namespace QPULib::Target::instr;

	// Setup DMA
	genSetReadPitch(seq, 4);
	genSetupDMALoad(seq, 16, 1, 1, 1, QPU_ID);
	// Start DMA load
	genStartDMALoad(seq, srcReg(e.deref.ptr->var));
	// Wait for DMA
	genWaitDMALoad(seq);
	// Setup VPM
	genSetupVPMLoad(seq, 1, QPU_ID, 0, 1);
	// Get from VPM
	Reg data(SPECIAL, SPECIAL_VPM_READ);
	seq->append(genLShift(dstReg(v), data, 0));
}


void SourceTranslate::setupVPMWriteStmt(Seq<Instr>* seq, Stmt *s) {
    QPULib::setupVPMWriteStmt(seq,
      s->setupVPMWrite.addr,
      s->setupVPMWrite.hor,
      s->setupVPMWrite.stride);
}


// ============================================================================
// Store request
// ============================================================================

// A 'store' operation of data to addr is almost the same as
// *addr = data.  The difference is that a 'store' waits until
// outstanding DMAs have completed before performing a write rather
// than after a write.  This enables other operations to happen in
// parallel with the write.

void SourceTranslate::storeRequest(Seq<Instr>* seq, Expr* data, Expr* addr) {
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

void SourceTranslate::regAlloc(CFG* cfg, Seq<Instr>* instrs) {
	vc4::regAlloc(cfg, instrs);
}

void SourceTranslate::add_init(Seq<Instr> &code) {
/*
	using namespace QPULib::Target::instr;

	int insert_index = get_init_begin_marker(code);

	Seq<Instr> ret;

	// When DMA is used, the index number is compensated for automatically, hence no need for it
	// offset = 4 * ( 16 * qpu_num);
	ret << shl(ACC0, rf(RSV_QPU_ID), 4 + 2);
	//ret << shl(ACC0, QPU_ID, 4 + 2);
	ret << add_uniform_pointer_offset(code);

	code.insert(insert_index + 1, ret);  // Insert init code after the INIT_BEGIN marker
*/
}

}  // namespace vc4
}  // namespace QPULib
