#include "SourceTranslate.h"
#include "../Support/debug.h"
#include "../Target/LoadStore.h"
#include "../Source/Translate.h"  // srcReg()


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
	assert(seq != nullptr);
	assert(rhs != nullptr);

	// QPU id
	Reg qpuId;
	qpuId.tag = SPECIAL;
	qpuId.regId = SPECIAL_QPU_NUM;
	// Setup VPM
	Reg addr = freshReg();
	seq->append(genLI(addr, 16));
	seq->append(genADD(addr, addr, qpuId));
	genSetupVPMStore(seq, addr, 0, 1);
	// Store address
	Reg storeAddr = freshReg();
	seq->append(genLI(storeAddr, 256));
	seq->append(genADD(storeAddr, storeAddr, qpuId));
	// Setup DMA
	genSetWriteStride(seq, 0);
	genSetupDMAStore(seq, 16, 1, 1, storeAddr);
	// Put to VPM
	Reg data;
	data.tag = SPECIAL;
	data.regId = SPECIAL_VPM_WRITE;
	seq->append(genLShift(data, srcReg(rhs->var), 0));
	// Start DMA
	genStartDMAStore(seq, srcReg(lhs.deref.ptr->var));
	// Wait for store to complete
	genWaitDMAStore(seq);

	return true;
}


void SourceTranslate::setupVPMWriteStmt(Seq<Instr>* seq, Stmt *s) {
    QPULib::setupVPMWriteStmt(seq,
      s->setupVPMWrite.addr,
      s->setupVPMWrite.hor,
      s->setupVPMWrite.stride);
}

}  // namespace vc4
}  // namespace QPULib
