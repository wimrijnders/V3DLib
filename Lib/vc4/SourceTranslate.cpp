#include "SourceTranslate.h"
#include "Support/debug.h"
#include "Source/Translate.h"  // srcReg()
#include "Target/Subst.h"
#include "DMA/LoadStore.h"
#include "RegAlloc.h"

namespace V3DLib {
namespace vc4 {

Instr::List SourceTranslate::deref_var_var(Var lhs, Var rhs) {
  return DMA::StoreRequest(lhs, rhs);
}


void SourceTranslate::varassign_deref_var(Instr::List *seq, Var &v, Expr &e) {
  *seq << DMA::varassign_deref_var(v, e);
}


void SourceTranslate::regAlloc(CFG* cfg, Instr::List *instrs) {
  vc4::regAlloc(cfg, instrs);
}


/**
 * @return true if statement handled, false otherwise
 */
bool SourceTranslate::stmt(Instr::List &seq, Stmt::Ptr s) {
  bool ret = true;

  switch (s->tag) {
    // Add tag handling here as required

    default:
      if (!DMA::translate_stmt(seq, s)) {
        assertq(false, "translate_stmt(): unexpected stmt tag", true);
        ret = false;
      }
      break;
  }

  return ret;
}

}  // namespace vc4
}  // namespace V3DLib
