#include "Instr.h"         // Location of definition struct Instr
#include "Support/debug.h"
#include "Target/Pretty.h"  // pretty_instr_tag()
#include "Support/basics.h" // fatal()
#include "Support/Platform.h"
#include "Source/BExpr.h"   // class CmpOp
#include "Target/SmallLiteral.h"

namespace V3DLib {

// ============================================================================
// Class RegOrImm
// ============================================================================

Reg &RegOrImm::reg()           { assert(is_reg()); return m_reg; }
Reg RegOrImm::reg() const      { assert(is_reg()); return m_reg; }
SmallImm &RegOrImm::imm()      { assert(is_imm()); return m_smallImm; }
SmallImm RegOrImm::imm() const { assert(is_imm()); return m_smallImm; }

void RegOrImm::set_imm(int rhs) {
  m_is_reg  = false;
  m_smallImm.val = rhs;
}


void RegOrImm::set_reg(RegTag tag, RegId id) {
  m_is_reg  = true;
  m_reg.tag   = tag;
  m_reg.regId = id;
}


void RegOrImm::set_reg(Reg const &rhs) {
  m_is_reg  = true;
  m_reg = rhs;
}

bool RegOrImm::operator==(RegOrImm const &rhs) const {
  if (m_is_reg != rhs.m_is_reg) return false;

  if (m_is_reg) {
    return m_reg == rhs.m_reg;
  } else {
    return m_smallImm == rhs.m_smallImm;
  }
}


std::string RegOrImm::disp() const {
  if (m_is_reg) {
    return m_reg.dump();
  } else {
    return printSmallLit(m_smallImm.val);
  }
}


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
 * Check that write is non-conditional
 *
 * TODO check if this is the exact complement of isCondAssign()
 */
bool Instr::is_always() const {
  bool always = (tag == InstrTag::LI && LI.cond.is_always())
             || (tag == InstrTag::ALU && ALU.cond.is_always());

  return always;
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

  if (!ALU.srcA.is_reg() || !ALU.srcB.is_reg()) {
    return false;  // Both operands must be regs
  }

  Reg aReg  = ALU.srcA.reg();
#ifdef DEBUG
  Reg bReg  = ALU.srcB.reg();
#endif

  if (aReg.tag == SPECIAL && aReg.regId == SPECIAL_UNIFORM) {
    assert(aReg == bReg);  // Apparently, this holds (NOT TRUE)
    return true;
  } else {
    assert(!(bReg.tag == SPECIAL && bReg.regId == SPECIAL_UNIFORM));  // not expecting this to happen
    return false;
  }
}


bool Instr::isUniformPtrLoad() const {
 return isUniformLoad() && (ALU.srcA.is_reg() && ALU.srcA.reg().isUniformPtr);
}


bool Instr::isTMUAWrite(bool fetch_only) const {
   if (tag != InstrTag::ALU) {
    return false;
  }

  Reg reg = ALU.dest;
  if (reg.tag != SPECIAL) {
    return false;
  }

  return (!fetch_only && reg.regId == SPECIAL_DMA_ST_ADDR)
      || (reg.regId == SPECIAL_TMU0_S);
}


bool Instr::isRot() const {
  if (tag != InstrTag::ALU) {
    return false;
  }

  return ALU.op.isRot();
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
      && LI.imm.is_zero()
  ;
}


/**
 * Can't be inlined for debugger
 */
std::string Instr::dump() const {
  return mnemonic(true);
}


/**
 * Determine the accumulators used in this instruction
 *
 * @return bitfield with the bits set for used accumulators,
 *         bit 0 == ACC0, bit 1 == ACC1 etc.
 */
uint32_t Instr::get_acc_usage() const {
  // No distinguishing dst and src here
  uint32_t ret = 0;

  switch (tag) {
    case InstrTag::TMU0_TO_ACC4:  // Load immediate
      ret |=  (1 << 4);
      break;

    case InstrTag::LI:  // Load immediate
      if (LI.dest.tag == ACC) {
        ret |=  (1 << LI.dest.regId);
      }
      break;

    case InstrTag::ALU:  // ALU operation
      if (ALU.dest.tag == ACC) {
        ret |=  (1 << ALU.dest.regId);
      }

      // NOTE: dst/srcA/srcB can be same acc

      if (ALU.srcA.is_reg() && ALU.srcA.reg().tag == ACC) {
        ret |=  (1 << ALU.srcA.reg().regId);
      }

      if (ALU.srcB.is_reg() && ALU.srcB.reg().tag == ACC) {
        ret |=  (1 << ALU.srcB.reg().regId);
      }
      break;

    case InstrTag::RECV:  // RECV instruction
      if (RECV.dest.tag == ACC) {
        ret |=  (1 << RECV.dest.regId);
      }
      break;

    default:
      break;
  }

  // ACC 0 and 1 are used rot v3d, add
  // dst r1 and src r0 might be explicitly set beforehand, this is fine.
  // Generation of rot-instruction checks for this
  if (!Platform::compiling_for_vc4()) {
    if (isRot()) {
      ret |= 3;
    }
  }

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Class Instr::List
///////////////////////////////////////////////////////////////////////////////

std::string Instr::List::dump(bool with_line_numbers) const {
  std::string ret;

  for (int i = 0; i < size(); ++i ) {
    if (with_line_numbers) {
      ret << i << ": ";
    }

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


/**
 * Debug function for displaying the used accumulators in the instruction list
 */
std::string Instr::List::check_acc_usage() const {
  std::string ret;

  for (int index = 0; index < size(); ++index) {
    uint32_t accs = (*this)[index].get_acc_usage();
    assert(accs < 64);

    if (accs == 0) continue;

    ret << "Instruction " << index << " uses ACCs: ";

    if (accs &  1) ret << "0, ";
    if (accs &  2) ret << "1, ";
    if (accs &  4) ret << "2, ";
    if (accs &  8) ret << "3, ";
    if (accs & 16) ret << "4, ";
    if (accs & 32) ret << "5, ";

    if ((*this)[index].isRot()) {
      ret << " - Rot instruction";
    }

    ret << "\n";
  }

  return ret;
}


/**
 * Return index of accumulator which is free for the given
 * range in the instruction list.
 *
 * If none can be found, return -1.
 */
int Instr::List::get_free_acc(int first, int last) const {
  assert(first <= last);
  assert(first >= 0);
  assert(last  < size());

  uint32_t acc_use = 0xffffffff;  // Keeps track of free acc's, default all free

  for (int i = first; i <= last; ++i) {
    acc_use = acc_use & ~(*this)[i].get_acc_usage();  // Remember, get_acc_usage() returns *used* acc's
  }

  // Mask out unused bits and also r5, because it has special usage
  // Actually, r3 (sfu) and r4 (tmu read) have special usages as well, 
  // but let's see how far we get.
  acc_use = acc_use & 0x1f;

  // Determine first non-zero bit
  int ret = -1;

  for (int i = 0; i < 5; ++i) {
    if ((acc_use & (1 << i)) != 0) {
      ret = i;
      break;
    }
  }

  return ret;
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
void check_zeroes(Instr::List const &instrs) {
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
    ret << emit_comment((int) (out.size() + prefix.size()));
  }

  return ret;
}

}  // namespace V3DLib
