#include "Instr.h"         // Location of definition struct Instr
#include "Target/Pretty.h"  // pretty_instr_tag()
#include "Support/basics.h" // fatal()
#include "Source/BExpr.h"   // class CmpOp

namespace V3DLib {

// ============================================================================
// Class BranchTarget
// ============================================================================

std::string BranchTarget::to_string() const {
  std::string ret;

  if (relative) ret << "PC+1+";
  if (useRegOffset) ret << "A" << regOffset << "+";
  ret << immOffset;

  return ret;
}


// ============================================================================
// Class Instr - QPU instructions
// ============================================================================

/**
 * Initialize the fields per selected instruction tag.
 *
 * Done like this, because union members can't have non-trivial constructors.
 */
Instr::Instr(InstrTag in_tag) {
  switch (in_tag) {
  case InstrTag::ALU:
    tag          = InstrTag::ALU;
    ALU.m_setCond.clear();
    ALU.cond     = always;
    break;
  case InstrTag::LI:
    tag          = InstrTag::LI;
    LI.m_setCond.clear();
    LI.cond      = always;
    break;

  case InstrTag::INIT_BEGIN:
  case InstrTag::INIT_END:
  case InstrTag::RECV:
  case InstrTag::PRI:
  case InstrTag::END:
  case InstrTag::TMU0_TO_ACC4:
    tag = in_tag;
    break;
  default:
    assert(false);
    break;
  }
}


Instr Instr::nop() {
  Instr instr;
  instr.tag = NO_OP;
  return instr;
}


/**
 * Initial capital to discern it from member var's `setFlags`.
 */
Instr &Instr::setCondFlag(Flag flag) {
  setCond().setFlag(flag);
  return *this;
}


Instr &Instr::setCondOp(CmpOp const &cmp_op) {
  setCond().tag(cmp_op.cond_tag());
  return *this;
}


Instr &Instr::cond(AssignCond in_cond) {
  ALU.cond = in_cond;
  return *this;
}


/**
 * Determine if instruction is a conditional assignment
 */
bool Instr::isCondAssign() const {
  if (tag == InstrTag::LI && !LI.cond.is_always())
    return true;

  if (tag == InstrTag::ALU && !ALU.cond.is_always())
    return true;

  return false;
}


/**
 * Determine if this is the last instruction in a basic block
 *
 * TODO Unused, do we need this?
 */
bool Instr::isLast() const {
  return tag == V3DLib::BRL || tag == V3DLib::BR || tag == V3DLib::END;
}


SetCond const &Instr::setCond() const {
  switch (tag) {
    case InstrTag::LI:
      return LI.m_setCond;
    case InstrTag::ALU:
      return ALU.m_setCond;
    default:
      assertq(false, "setCond() can only be called for LI or ALU");
      break;
  }

  return ALU.m_setCond;  // Return anything
}


SetCond &Instr::setCond() {
  switch (tag) {
    case InstrTag::LI:
      return LI.m_setCond;
    case InstrTag::ALU:
      return ALU.m_setCond;
    default:
      assertq(false, "setCond() can only be called for LI or ALU");
      break;
  }

  return ALU.m_setCond;  // Return anything
}


Instr &Instr::pushz() {
  setCond().tag(SetCond::Z);
  return *this;
}


/**
 * Convert branch label to branch target
 * 
 * @param offset  offset to the label from current instruction
 */
void Instr::label_to_target(int offset) {
  assert(tag == InstrTag::BRL);

  // Convert branch label (BRL) instruction to branch instruction with offset (BR)
  // Following assumes that BranchCond field 'cond' survives the transition to another union member

  BranchTarget t;
  t.relative       = true;
  t.useRegOffset   = false;
  t.immOffset      = offset - 4;  // Compensate for the 4-op delay for executing a branch

  tag        = InstrTag::BR;
  BR.target  = t;
}



bool Instr::isUniformLoad() const {
  if (tag != InstrTag::ALU) {
    return false;
  }

  if (ALU.srcA.tag != REG || ALU.srcB.tag != REG) {
    return false;  // Both operands must be regs
  }

  Reg aReg  = ALU.srcA.reg;
#ifdef DEBUG
  Reg bReg  = ALU.srcB.reg;
#endif

  if (aReg.tag == SPECIAL && aReg.regId == SPECIAL_UNIFORM) {
    assert(aReg == bReg);  // Apparently, this holds (NOT TRUE)
    return true;
  } else {
    assert(!(bReg.tag == SPECIAL && bReg.regId == SPECIAL_UNIFORM));  // not expecting this to happen
    return false;
  }
}


bool Instr::isTMUAWrite(bool fetch_only) const {
   if (tag != InstrTag::ALU) {
    return false;
  }

  Reg reg = ALU.dest;
  if (reg.tag != SPECIAL) {
    return false;
  }

  bool ret = (!fetch_only && reg.regId == SPECIAL_DMA_ST_ADDR)
          || (reg.regId == SPECIAL_TMU0_S);

#if 0
  // Prob not needed, triple check before removing (already single-checked)

  if (ret) {
    // It's a simple move (BOR) instruction, src registers should be the same
    auto reg_a = ALU.srcA;
    auto reg_b = ALU.srcB;
    std::string msg = "src registers should be the same; instruction:";
    assertq(reg_a == reg_b, msg << mnemonic(true) , true);

/*
    // In current logic, src should always be read from register file;
    // enforce this.
    if (!(reg_a.tag == REG && (reg_a.reg.tag == REG_A || reg_a.reg.tag == REG_B))) {
      std::string msg = "Not allowed by logic; instruction: ";
      assertq(false, msg << mnemonic(true)); 
    }
*/
  }
#endif

  return ret;
}


/**
 * Determine if this instruction has all fields set to zero.
 *
 * This is an illegal instruction, but has popped up.
 */
bool Instr::isZero() const {
  return tag == InstrTag::LI
      && !LI.m_setCond.flags_set()
      && LI.cond.tag      == AssignCond::NEVER
      && LI.cond.flag     == ZS
      && LI.dest.tag      == REG_A
      && LI.dest.regId    == 0
      && LI.dest.isUniformPtr == false 
      && LI.imm.tag       == IMM_INT32
      && LI.imm.intVal    == 0
  ;
}


/**
 * Can't be inlined for debugger
 */
std::string Instr::dump() const {
  return mnemonic(true);
}


///////////////////////////////////////////////////////////////////////////////
// Class Instr::List
///////////////////////////////////////////////////////////////////////////////

std::string Instr::List::dump() const {
  std::string ret;

  for (int i = 0; i < size(); ++i ) {
    ret << (*this)[i].dump() << "\n";
  }

  return ret;
}


/**
 * Generates a string representation of the passed string of instructions.
 */
std::string Instr::List::mnemonics(bool with_comments) const {
  std::string prefix;
  std::string ret;

  for (int i = 0; i < size(); i++) {
    auto const &instr = (*this)[i];
    prefix.clear();
    prefix << i << ": ";

    ret << instr.mnemonic(with_comments, prefix).c_str() << "\n";
  }

  return ret;
}


/**
 * Get the index of the last uniform load
 */
int Instr::List::lastUniformOffset() {
  // Determine the first offset that is not a uniform load
  int index = 0;
  for (; index < size(); ++index) {
    if (!(*this)[index].isUniformLoad()) break; 
  }

  assertq(index >= 2, "Expecting at least two uniform loads.", true);
  return index - 1;
}


int Instr::List::tag_count(InstrTag tag) {
  int count = 0;

  for (int index = 0; index < size(); ++index) {
    if ((*this)[index].tag == tag) {
      ++count;
    }
  }

  return count;
}


int Instr::List::tag_index(InstrTag tag, bool ensure_one) {
  // Find the init begin marker
  int found = -1;
  int count = 0;

  for (int index = 0; index < size(); ++index) {
    if ((*this)[index].tag == tag) {
      if (found == -1) {  // Only remember first
        found = index;
      }
      count++;
    }
  }

  assertq(!ensure_one || count == 1, "List::tag_index() Expecting exactly one tag found.");
  return found;
}

///////////////////////////////////////////////////////////////////////////////
// End Class Instr::List
///////////////////////////////////////////////////////////////////////////////

/**
 * Check if given tag is for the specified platform
 */
void check_instruction_tag_for_platform(InstrTag tag, bool for_vc4) {
  char const *platform = nullptr;

  if (for_vc4) {
    if (tag >= V3D_ONLY && tag < END_V3D_ONLY) {
      platform = "vc4";
    } 
  } else {  // v3d
    if (tag >= VC4_ONLY && tag < END_VC4_ONLY) {
      platform = "v3d";
    }
  }

  if (platform != nullptr) {
    std::string msg = "Instruction tag ";
    msg << pretty_instr_tag(tag) << "(" + std::to_string(tag) + ")" << " can not be used on " << platform;
    fatal(msg);
  }
}


/**
 * Debug function - check for presence of zero-instructions in instruction sequence
 *
 */
void check_zeroes(Seq<Instr> const &instrs) {
  bool success = true;

  for (int i = 0; i < instrs.size(); ++i ) {
    if (instrs[i].isZero()) {
      std::string msg = "Zero instruction encountered at position ";
      // Grumbl not working:  msg << i;
      msg += std::to_string(i);
      warning(msg.c_str());

      success = false;
    }
  }

  if (!success) {
    error("zeroes encountered in instruction sequence", true);
  }
}


/**
 * Returns a string representation of an instruction.
 */
std::string Instr::mnemonic(bool with_comments, std::string const &prefix) const {
  std::string ret;

  if (with_comments) {
    ret << emit_header();
  }

  std::string out = pretty_instr(*this);
  ret << prefix << out;

  if (with_comments) {
    ret << emit_comment((int) out.size());
  }

  return ret;
}

}  // namespace V3DLib
