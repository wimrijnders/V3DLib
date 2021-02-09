#include "SourceTranslate.h"
#include "Support/debug.h"
#include "Target/Syntax.h"
#include "Target/Subst.h"
#include "DMA/LoadStore.h"
#include "RegAlloc.h"
#include "LibSettings.h"

namespace V3DLib {
namespace vc4 {

Instr::List SourceTranslate::load_var(Var &in_dst, Expr &e) {
  using namespace Target::instr;

  if (LibSettings::use_tmu_for_load()) {
    //warning("Using TMU for var load");
    Instr::List ret;

    Reg src = srcReg(e.deref_ptr()->var());
    Reg dst = dstReg(in_dst);

    // Compare with RECV handling in:
    // - loadStorePass(),
    // - gather/receive
    // - SourceTranslate::load_var(0 for v3d
    ret << mov(ACC0, ELEM_ID)
        << shl(ACC0, ACC0, 2)
        << add(ACC0, ACC0, src)

        << mov(TMU0_S, ACC0)
        << Instr(TMU0_TO_ACC4)
        << mov(dst, ACC4);

    return ret;
  } else {
    //warning("Using DMA for var load");
    return DMA::loadRequest(in_dst, e);
  }
}


Instr::List SourceTranslate::store_var(Var dst_addr, Var src) {
  return DMA::storeRequest(dst_addr, src);
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
      if (!DMA::translate_stmt(seq, s->tag, s->dma)) {
        assertq(false, "translate_stmt(): unexpected stmt tag", true);
        ret = false;
      }
      break;
  }

  return ret;
}

}  // namespace vc4
}  // namespace V3DLib
