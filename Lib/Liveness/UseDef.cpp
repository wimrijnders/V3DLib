#include "UseDef.h"

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class UseDefReg
///////////////////////////////////////////////////////////////////////////////

bool UseDefReg::is_dest(Reg const &rhs) const {
  for (int j = 0; j < def.size(); j++) {
    if (def[j] == rhs) return true;
  }

  return false;
}


bool UseDefReg::is_src(Reg const &rhs) const {
  for (int j = 0; j < use.size(); j++) {
    if (use[j] == rhs) return true;
  }

  return false;
}


std::string UseDefReg::dump() const {
  std::string ret;

  ret << "(def: ";
  for (int j = 0; j < def.size(); j++) {
    ret << def[j].dump() << ", ";
  }
  ret << "; ";

  ret << "use: ";
  for (int j = 0; j < use.size(); j++) {
    ret << use[j].dump() << ", ";
  }
  ret << ") ";

  return ret;
}


/**
 * Compute 'use' and 'def' sets for a given instruction
 *
 * Param 'set_use_where' need only be true during liveness analysis.
 *
 * @param set_use_where  if true, regard assignments in conditional 'where'
 *                       instructions as usage.
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * 'set_use_where' needs to be true for the following case (target language):
 *
 *    LI A5 <- 0                  # assignment
 *    ...
 *    where ZC: LI A6 <- 1
 *    where ZC: A5 <- or(A6, A6)  # Conditional assignment
 *    ...
 *    S[VPM_WRITE] <- shl(A5, 0)  # last use
 *
 *   If the condition is ignored (`set_use_where == false`), the conditional
 *   assignment is regarded as an overwrite of the previous one. The variable
 *   is then considered live from the conditional assignment onward.
 *   This is wrong, the value of the first assignment may be significant due
 *   to the condition. The usage of `A5` runs the risk of being assigned different
 *   registers for the different assignments, which will lead to wrong code execution.
 *
 * * However, always using `set_use_where == true` leads to variables being live
 *   for unnecessarily long. If this is the *only* usage of `A6`:
 *
 *    where ZC: LI A6 <- 1
 *    where ZC: A5 <- or(A6, A6)  # Conditional assignment
 *
 *   ... `A6` would be considered live from the start of the program
 *   onward till the last usage.
 *   This unnecessarily ties up a register for a long duration, complicating the
 *   allocation by creating a false shortage of registers.
 *   This case can not be handled by the liveness analysis as implemented here.
 *   It is corrected afterwards in methode `Liveness::compute()`.
 */
void UseDefReg::set_used(Instr instr, bool set_use_where) {
  auto ALWAYS = AssignCond::Tag::ALWAYS;

  use.clear();
  def.clear();

  switch (instr.tag) {
    case LI:                                     // Load immediate
      def.insert(instr.LI.dest);                 // Add destination reg to 'def' set

      if (set_use_where) {
        if (instr.LI.cond.tag != ALWAYS)         // Add destination reg to 'use' set if conditional assigment
          use.insert(instr.LI.dest);
      }
      return;

    case ALU:                                    // ALU operation
      def.insert(instr.ALU.dest);                // Add destination reg to 'def' set

      if (set_use_where) {
        if (instr.ALU.cond.tag != ALWAYS)        // Add destination reg to 'use' set if conditional assigment
          use.insert(instr.ALU.dest);
      }

      if (instr.ALU.srcA.is_reg())               // Add source reg A to 'use' set
        use.insert(instr.ALU.srcA.reg());

      if (instr.ALU.srcB.is_reg())               // Add source reg B to 'use' set
        use.insert(instr.ALU.srcB.reg());
      return;

    case RECV:                                   // Load receive instruction
      def.insert(instr.RECV.dest);               // Add dest reg to 'def' set
      return;
    default:
      return;
  }  
}


///////////////////////////////////////////////////////////////////////////////
// Class UseDef
///////////////////////////////////////////////////////////////////////////////

std::string UseDef::dump() const {
  std::string ret;

  ret << "(def: ";
  for (int j = 0; j < def.size(); j++) {
    ret << def[j] << ", ";
  }
  ret << "; ";

  ret << "use: ";
  for (int j = 0; j < use.size(); j++) {
    ret << use[j] << ", ";
  }
  ret << ") ";

  return ret;
}


/**
 * Get variables used in instruction
 *
 * Same as `useDefReg()`, except only yields ids of registers in register file A.
 */
void UseDef::set_used(Instr const &instr, bool set_use_where) {
  UseDefReg set;
  set.set_used(instr, set_use_where);

  use.clear();
  def.clear();

  for (int i = 0; i < set.use.size(); i++) {
    Reg r = set.use[i];
    if (r.tag == REG_A) use.insert(r.regId);
  }

  for (int i = 0; i < set.def.size(); i++) {
    Reg r = set.def[i];
    if (r.tag == REG_A) def.insert(r.regId);
  }
}

}  // namespace V3DLib
