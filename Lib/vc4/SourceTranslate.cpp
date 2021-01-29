#include "SourceTranslate.h"
#include "Support/debug.h"
#include "Source/Translate.h"  // srcReg()
#include "Target/Subst.h"
#include "DMA/LoadStore.h"
#include "DMA/Translate.h"
#include "RegAlloc.h"

namespace V3DLib {
namespace vc4 {

Instr::List SourceTranslate::deref_var_var(Var lhs, Var rhs) {
  Instr::List ret;
  
  ret << StoreRequest(lhs, rhs)
      << genWaitDMAStore();  // Wait for store to complete

  return ret;
}


void SourceTranslate::varassign_deref_var(Instr::List *seq, Var &v, Expr &e) {
  using namespace V3DLib::Target::instr;

  Reg reg = srcReg(e.deref_ptr()->var());
  Instr::List ret;
  
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


void SourceTranslate::regAlloc(CFG* cfg, Instr::List *instrs) {
  vc4::regAlloc(cfg, instrs);
}


/**
 * @return true if statement handled, false otherwise
 */
bool SourceTranslate::stmt(Instr::List &seq, Stmt::Ptr s) {
  return vc4::translate_stmt(seq, s);
}

}  // namespace vc4
}  // namespace V3DLib
