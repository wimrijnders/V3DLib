#include "Target/Subst.h"

namespace V3DLib {

/**
 * Rename a destination register in an instruction
 *
 * @return number of substitutions performed.
 */
int renameDest(Instr &instr, Reg const &current, Reg const &replace_with) {
  int count = 0;

  switch (instr.tag) {
    case LI:  // Load immediate
      if (instr.LI.dest == current) {
        instr.LI.dest = replace_with;
        count += 1;
      }
      break;

    case ALU:  // ALU operation
      if (instr.ALU.dest == current) {
        instr.ALU.dest = replace_with;
        count += 1;
      }
      break;

    case RECV:  // RECV instruction
      if (instr.RECV.dest == current) {
        instr.RECV.dest = replace_with;
        count += 1;
      }
      break;

    default:
      break;
  }

  assert(count <= 1);
  return count;
}


/**
 * Rename a used register in an instruction
 */
void renameUses(Instr &instr, Reg const &current, Reg const &replace_with) {
  if  (instr.tag != ALU) return;

  if (instr.ALU.srcA.is_reg() && instr.ALU.srcA.reg == current) {
    instr.ALU.srcA.reg = replace_with;
  }

  if (instr.ALU.srcB.is_reg() && instr.ALU.srcB.reg == current) {
    instr.ALU.srcB.reg = replace_with;
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
