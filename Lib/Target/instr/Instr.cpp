#include "Instr.h"         // Location of definition struct Instr
#include "Support/debug.h"
#include "Target/Pretty.h"  // pretty_instr_tag()
#include "Support/basics.h"
#include "Support/Platform.h"
#include "Source/BExpr.h"   // class CmpOp
#include "LibSettings.h"

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
    tag          = in_tag;
    m_assign_cond = always;
    m_set_cond.clear();
    break;

  case InstrTag::LI:
    tag           = in_tag;
    m_assign_cond = always;
    m_set_cond.clear();
    break;

  case InstrTag::BRL:
    tag               = in_tag;
    m_branch_cond.tag = BranchCond::COND_ALWAYS;
    break;

  case InstrTag::INIT_BEGIN:
  case InstrTag::INIT_END:
  case InstrTag::RECV:
  case InstrTag::END:
  case InstrTag::VPM_STALL:
    tag = in_tag;
    break;

  default:
    assert(false);
    break;
  }
}


Reg Instr::dest() const {
  assertq(has_dest(), "oops", true);
  return m_dest;
}


void Instr::dest(Reg const &rhs) {
  assertq(has_dest(), "oops", true);
  m_dest = rhs;
}


Instr Instr::nop() {
  Instr instr;
  instr.tag = NO_OP;
  return instr;
}


/**
 * There is at most 1 dst register.
 *
 * Absence of it is indicated by tag NONE in the return value
 */
Reg Instr::dst_reg() const {
  if (has_dest()) return dest();
  return Reg(NONE, 0);
}


Reg Instr::dst_a_reg() const {
  if (!has_dest()) return Reg(NONE, 0);

  Reg ret = dest();
  if (ret.tag != REG_A) ret.tag = NONE;
  return ret;
}


RegIdSet Instr::src_a_regs(bool set_use_where) const {
  RegIdSet ret;

  for (auto const &r : src_regs(set_use_where)) {
    if (r.tag == REG_A) ret.insert(r.regId);
  }

  return ret;
}


bool Instr::is_dst_reg(Reg const &rhs) const {
  Reg dst = dst_reg();

  if (rhs.tag == NONE) {
    breakpoint
  }

  return (dst.tag != NONE && dst == rhs);
}


/**
 * Return all source registers in this instruction
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
std::set<Reg> Instr::src_regs(bool set_use_where) const {
  auto ALWAYS = AssignCond::Tag::ALWAYS;

  std::set<Reg> ret;

  if (set_use_where) {  // Add destination reg to 'use' set if conditional assigment
    if (tag == InstrTag::LI || tag == InstrTag::ALU) {
      if (m_assign_cond.tag != ALWAYS) {
        ret.insert(dest());
      }
    }
  }

  if (tag == InstrTag::ALU) {
    if (ALU.srcA.is_reg()) ret.insert(ALU.srcA.reg());
    if (ALU.srcB.is_reg()) ret.insert(ALU.srcB.reg());
  }  

  return ret;
}


bool Instr::is_src_reg(Reg const &rhs) const {
  if (tag != InstrTag::ALU) return false;

  if (ALU.srcA.is_reg() && ALU.srcA.reg() == rhs) return true;
  if (ALU.srcB.is_reg() && ALU.srcB.reg() == rhs) return true;

  return false;
}


/**
 * Rename a destination register in an instruction
 *
 * @return true if anything replaced, false otherwise
 */
bool Instr::rename_dest(Reg const &current, Reg const &replace_with) {
  assert(current != replace_with);  // Otherwise subst is senseless
  if (!has_dest()) return false;

  if (dest() == current) {
    dest(replace_with);
    return true;
  }

  return false;
}


Instr &Instr::setCondFlag(Flag flag) {
  assert(tag == InstrTag::LI || InstrTag::ALU);
  m_set_cond.setFlag(flag);
  return *this;
}


Instr &Instr::setCondOp(CmpOp const &cmp_op) {
  assert(tag == InstrTag::LI || InstrTag::ALU);
  m_set_cond.tag(cmp_op.cond_tag());
  return *this;
}


Instr &Instr::cond(AssignCond in_cond) {
  assign_cond(in_cond);
  return *this;
}


/**
 * Determine if instruction is a conditional assignment
 */
bool Instr::isCondAssign() const {
  if (tag == InstrTag::LI || tag == InstrTag::ALU) {
   return !m_assign_cond.is_always();
  }

  return false;
}


void Instr::assign_cond(AssignCond rhs) { assert(tag == InstrTag::LI || tag == InstrTag::ALU); m_assign_cond = rhs; }
AssignCond Instr::assign_cond() const   { assert(tag == InstrTag::LI || tag == InstrTag::ALU); return m_assign_cond; }

BranchTarget Instr::branch_target() const { assert(tag == V3DLib::BR); return m_branch_target; }

void  Instr::branch_label(Label rhs) { assert(tag == InstrTag::BRL); m_branch_label = rhs; }
Label Instr::branch_label() const    { assert(tag == InstrTag::BRL); return m_branch_label; }

Instr &Instr::branch_cond(BranchCond rhs) {
  assert(tag == V3DLib::BR || tag == V3DLib::BRL);
  m_branch_cond = rhs;
  return *this;
}


BranchCond Instr::branch_cond() const {
  assert(tag == V3DLib::BR || tag == V3DLib::BRL);
  return m_branch_cond;
}


/**
 * Check that write is non-conditional
 *
 * TODO check if this is the exact complement of isCondAssign()
 */
bool Instr::is_always() const {
  if (tag == InstrTag::LI || tag == InstrTag::ALU) {
    return m_assign_cond.is_always();
  }

  return true;  // TODO returned false previously, check correct working
}


/**
 * Determine if this is the last instruction in a basic block
 *
 * TODO Unused, do we need this?
 */
bool Instr::isLast() const {
  return tag == V3DLib::BRL || tag == V3DLib::BR || tag == V3DLib::END;
}


SetCond Instr::set_cond() const {
  assert(tag == InstrTag::LI || InstrTag::ALU);
  return m_set_cond;
}


Instr &Instr::pushz() {
  assert(tag == InstrTag::LI || InstrTag::ALU);
  m_set_cond.tag(SetCond::Z);
  return *this;
}

Instr &Instr::allzc() {
  assert(tag == InstrTag::BRL);
  m_branch_cond.tag  = BranchCond::COND_ALL;
  m_branch_cond.flag = Flag::ZC;
  return *this;
}


/**
 * Convert branch label to branch target
 * 
 * Convert branch label (BRL) instruction to branch instruction with offset (BR).
 * 
 * @param offset  offset to the label from current instruction
 */
void Instr::label_to_target(int offset) {
  assert(tag == InstrTag::BRL);

  BranchTarget t;
  t.relative       = true;
  t.useRegOffset   = false;
  t.immOffset      = offset - 4;  // Compensate for the 4-op delay for executing a branch

  tag = InstrTag::BR;
  m_branch_target = t;
}



bool Instr::isUniformLoad() const {
  if (tag != InstrTag::ALU) {
    return false;
  }

  Reg const UNIFORM_READ( SPECIAL, SPECIAL_UNIFORM);  // From Mnemonics

  if (ALU.srcA != UNIFORM_READ) {
    assertq(ALU.srcB != UNIFORM_READ, "Both srcA and srcB should both be UNIFORM_READ or not");  // Sanity check
    return false;
  }

  // Sanity checks
  assertq(ALU.srcB == UNIFORM_READ, "Both srcA and srcB should be UNIFORM_READ");
  assertq(ALU.op == ALUOp::A_BOR, "Expcting uniform read only in combination with move");  // This is how we use it, 
                                                                                           // may be overly strict.
  return true;
}


bool Instr::isUniformPtrLoad() const {
 return isUniformLoad() && (ALU.srcA.is_reg() && ALU.srcA.reg().isUniformPtr);
}


bool Instr::isRot() const {
  return (tag == InstrTag::ALU) && ALU.op.isRot();
}


/**
 * Determine if this instruction has all fields set to zero.
 *
 * This is an illegal instruction, but has popped up.
 */
bool Instr::isZero() const {
  return tag == InstrTag::LI
      && !m_set_cond.flags_set()
      && m_assign_cond == AssignCond(AssignCond::NEVER, ZS)
      && LI.imm.is_zero()
      && dest() == Reg(REG_A, 0) 
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
 * There is no distinguishing dst and src here.
 *
 * @return bitfield with the bits set for used accumulators,
 *         bit 0 == ACC0, bit 1 == ACC1 etc.
 *
 * ============================================================================
 * NOTES
 * =====
 *
 *
 * 1. Somewhat of a hack: LI for v3d can potentially use r0 and r1, flag as used here.
 *    See encode_int_immediate() and convert_int_powers() in v3d KernelDriver. 
 *    This could be further specified.
 *
 *    Better would be to:
 *     - Flag usage accs in v3d instruction generation
 *     - Allow more flexibility in v3d instruction generation to select accs
 *     - Don't use accs at all there, but that needs a way to select rf-regs during v3d generation
 *
 *    A bit unhappy about this, but it's necessary to prevent.
 *    Another brilliant idea (ie use accs in v3d instructions) which is turning out to be a brain fart.
 */
uint32_t Instr::get_acc_usage() const {
  uint32_t ret = 0;

  switch (tag) {
    case InstrTag::LI:  // Load immediate
      if (dest().tag == ACC) {
        ret |=  (1 << dest().regId);
      }

      if (!Platform::compiling_for_vc4()) {  // See Note 1.
        ret |= 3;  //debug("LI block acc0 and acc1");
      }

      break;

    case InstrTag::ALU:  // ALU operation
      if (dest().tag == ACC) {
        ret |=  (1 << dest().regId);
      }

      // NOTE: dst/srcA/srcB can be same acc

      if (ALU.srcA.is_reg() && ALU.srcA.reg().tag == ACC) {
        ret |=  (1 << ALU.srcA.reg().regId);
      }

      if (ALU.srcB.is_reg() && ALU.srcB.reg().tag == ACC) {
        ret |=  (1 << ALU.srcB.reg().regId);
      }

      if (ALU.op == ALUOp::A_FSIN) {
        if (!Platform::compiling_for_vc4()) {
          // SIN using special reg always returns result in r4
          assertq((ret & (1 << 4)) == 0, "get_acc_usage(): Not really expecting r4 to be already in use for sin");
          ret |=  (1 << 4);
        }
      }
      break;

    case InstrTag::RECV:
      if (dest().tag == ACC) {
        ret |=  (1 << dest().regId);
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
std::string Instr::List::check_acc_usage(int first, int last) const {
  if (first == -1) first = 0;
  if (last  == -1) last = size() -1;
  assert(first <= last);

  std::string ret;

  for (int index = first; index <= last; ++index) {
    uint32_t accs = (*this)[index].get_acc_usage();
    assert(accs < 64);

    if (accs == 0) continue;

    ret << index << ": ";

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

  // Mask out unused bits and also r5, because it has special usage.
  // NOTE, r3 (sfu) and r4 (tmu read) have special usages as well, 
  if (Platform::compiling_for_vc4()) {
    // It appears to be required for vc4 to not use r4 (unit test [cond] fails)
    // Translation:
    // Target     : LI ACC4 <- 0
    // vc4 opcodes: load_imm tmu_noswap, nop, 0x00000000 (0.000000)
    acc_use = acc_use & 0xf;   // r0-r3
  } else {
    acc_use = acc_use & 0x1f;  // r0-r4
  }

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

  if (!for_vc4) {
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
