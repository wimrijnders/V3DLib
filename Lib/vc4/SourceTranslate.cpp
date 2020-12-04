#include "SourceTranslate.h"
#include "Support/debug.h"
#include "Source/Translate.h"  // srcReg()
#include "LoadStore.h"
#include "Target/Subst.h"
#include "Translate.h"
#include "RegAlloc.h"

namespace V3DLib {
namespace vc4 {

Seq<Instr> SourceTranslate::deref_var_var(Var lhs, Var rhs) {
	Seq<Instr> ret;
	
	ret << StoreRequest(lhs, rhs)
	    << genWaitDMAStore();  // Wait for store to complete

	return ret;
}


void SourceTranslate::varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) {
	using namespace V3DLib::Target::instr;

	Reg reg = srcReg(e.deref_ptr()->var());
	Seq<Instr> ret;
	
	ret << genSetReadPitch(4)                           // Setup DMA
	    << genSetupDMALoad(16, 1, 1, 1, QPU_ID)
	    << genStartDMALoad(reg)                         // Start DMA load
	    << genWaitDMALoad(false)                        // Wait for DMA
	    << genSetupVPMLoad(1, QPU_ID, 0, 1)             // Setup VPM
	    << shl(dstReg(v), Target::instr::VPM_READ, 0);  // Get from VPM

	ret.front().comment("Start DMA load var");
	ret.back().comment("End DMA load var");

	*seq << ret;
}


void SourceTranslate::regAlloc(CFG* cfg, Seq<Instr>* instrs) {
	vc4::regAlloc(cfg, instrs);
}


/**
 * @return true if statement handled, false otherwise
 */
bool SourceTranslate::stmt(Seq<Instr> &seq, Stmt* s) {
	return vc4::translate_stmt(seq, s);
}

}  // namespace vc4
}  // namespace V3DLib
