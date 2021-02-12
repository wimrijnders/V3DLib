#include "Target/Subst.h"

namespace V3DLib {

/**
 * Rename a destination register in an instruction
 */
void renameDest(Instr* instr, RegTag vt, RegId v, RegTag wt, RegId w) {
  switch (instr->tag) {
    // Load immediate
    case LI:
      if (instr->LI.dest.tag == vt && instr->LI.dest.regId == v) {
        instr->LI.dest.tag = wt;
        instr->LI.dest.regId = w;
      }
      return;

    // ALU operation
    case ALU:
      if (instr->ALU.dest.tag == vt && instr->ALU.dest.regId == v) {
        instr->ALU.dest.tag = wt;
        instr->ALU.dest.regId = w;
      }
      return;

    // RECV instruction
    case RECV:
      if (instr->RECV.dest.tag == vt && instr->RECV.dest.regId == v) {
        instr->RECV.dest.tag = wt;
        instr->RECV.dest.regId = w;
      }
      return;
    default:
      return;
  }
}


/**
 * Renamed a used register in an instruction
 */
void renameUses(Instr* instr, RegTag vt, RegId v, RegTag wt, RegId w) {
  switch (instr->tag) {
    case ALU:   // ALU operation
      if (instr->ALU.srcA.is_reg() && instr->ALU.srcA.reg.tag == vt &&
          instr->ALU.srcA.reg.regId == v) {
        instr->ALU.srcA.reg.tag = wt;
        instr->ALU.srcA.reg.regId = w;
      }

      if (instr->ALU.srcB.is_reg() && instr->ALU.srcB.reg.tag == vt &&
          instr->ALU.srcB.reg.regId == v) {
        instr->ALU.srcB.reg.tag = wt;
        instr->ALU.srcB.reg.regId = w;
      }
      return;

    default:
      return;
  }
}


/**
 * Globally change register tag vt to wt in given instruction
 */
void substRegTag(Instr* instr, RegTag vt, RegTag wt) {
  switch (instr->tag) {
    case LI:  // Load immediate
      if (instr->LI.dest.tag == vt)
        instr->LI.dest.tag = wt;
      return;

    case ALU:
      if (instr->ALU.dest.tag == vt)
        instr->ALU.dest.tag = wt;
      if (instr->ALU.srcA.is_reg() && instr->ALU.srcA.reg.tag == vt)
        instr->ALU.srcA.reg.tag = wt;
      if (instr->ALU.srcB.is_reg() && instr->ALU.srcB.reg.tag == vt)
        instr->ALU.srcB.reg.tag = wt;
      return;

    case RECV:
      if (instr->RECV.dest.tag == vt)
        instr->RECV.dest.tag = wt;
      return;
    default:
      return;
  }
}

}  // namespace V3DLib
