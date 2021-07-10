#include "Target/Subst.h"

namespace V3DLib {



/**
 * Rename used registers in an instruction
 *
 * @return true if anything replaced, false otherwise
 */
bool renameUses(Instr &instr, Reg const &current, Reg const &replace_with) {
  assert(current != replace_with);  // otherwise rename is senseless

  if  (instr.tag != ALU) return false;
  bool replaced = false;

  if (instr.ALU.srcA.is_reg() && instr.ALU.srcA.reg() == current) {
    instr.ALU.srcA.reg() = replace_with;
    replaced = true;
  }

  if (instr.ALU.srcB.is_reg() && instr.ALU.srcB.reg() == current) {
    instr.ALU.srcB.reg() = replace_with;
    replaced = true;
  }

  return replaced;
}


/**
 * Globally change register tag vt to wt in given instruction
 */
void substRegTag(Instr* instr, RegTag vt, RegTag wt) {
  assert(vt != wt);  // otherwise subst is senseless

  if (instr->has_dest()) {
    Reg tmp = instr->dest();
    if (tmp.tag == vt) {
      tmp.tag = wt;
      instr->dest(tmp);
    }
  }

  if (instr->tag == ALU) {
      if (instr->ALU.srcA.is_reg() && instr->ALU.srcA.reg().tag == vt)
        instr->ALU.srcA.reg().tag = wt;

      if (instr->ALU.srcB.is_reg() && instr->ALU.srcB.reg().tag == vt)
        instr->ALU.srcB.reg().tag = wt;
  }
}

}  // namespace V3DLib
