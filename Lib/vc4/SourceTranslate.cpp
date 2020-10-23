#include "SourceTranslate.h"
#include "Support/debug.h"
#include "Source/Translate.h"  // srcReg()
#include "LoadStore.h"
#include "Target/Subst.h"
#include "Translate.h"
#include "RegAlloc.h"

namespace QPULib {
namespace vc4 {

bool SourceTranslate::deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr *rhs) {
	assert(seq != nullptr);
	assert(rhs != nullptr);

	StoreRequest(*seq, lhs.deref.ptr->var, rhs->var);
	*seq << genWaitDMAStore();  // Wait for store to complete
	return true;
}


void SourceTranslate::varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) {
	using namespace QPULib::Target::instr;

	*seq << genSetReadPitch(4).comment("Start DMA load var")                         // Setup DMA
	     << genSetupDMALoad(16, 1, 1, 1, QPU_ID)
	     << genStartDMALoad(srcReg(e.deref.ptr->var))                                // Start DMA load
	     << genWaitDMALoad(false)                                                    // Wait for DMA
	     << genSetupVPMLoad(1, QPU_ID, 0, 1)                                         // Setup VPM
	     << shl(dstReg(v), Target::instr::VPM_READ, 0).comment("End DMA load var");  // Get from VPM
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


/**
 * @return true if statement handled, false otherwise
 */
bool SourceTranslate::stmt(Seq<Instr>* seq, Stmt* s) {
	return vc4::stmt(seq, s);
}

}  // namespace vc4
}  // namespace QPULib
