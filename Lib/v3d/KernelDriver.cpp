#include "KernelDriver.h"
#include <iostream>
#include <memory>
#include "Driver.h"
#include "Source/Translate.h"
#include "Target/SmallLiteral.h"  // decodeSmallLit()
#include "Target/RemoveLabels.h"
#include "instr/Snippets.h"
#include "Support/basics.h"
#include "Support/Timer.h"
#include "SourceTranslate.h"

namespace V3DLib {

// WEIRDNESS, due to changes, this file did not compile because it suddenly couldn't find
// the relevant overload of operator <<.
// Adding this solved it. HUH??
// Also: the << definitions in `basics.h` DID get picked up; the std::string versions did not.
using ::operator<<; // C++ weirdness

namespace v3d {

using namespace V3DLib::v3d::instr;
using Instructions = V3DLib::v3d::Instructions;

namespace {

uint8_t const NOP_ADDR    = 39;
uint8_t const REGB_OFFSET = 32;
uint8_t const NUM_REGS_RF = 64;  // Number of available registers in the register file

std::vector<std::string> local_errors;


void check_reg(Reg reg) {
  if (reg.regId < 0) {
    error("Unassigned regId value", true);
  }

  if (reg.regId >= NUM_REGS_RF) {
    breakpoint
    error("regId value out of range", true);
  }
}


uint8_t to_waddr(Reg const &reg) {
  assertq(reg.tag != REG_B, "to_waddr(): Not expecting REG_B any more, examine");
  assert(reg.tag == REG_A);
  return (uint8_t) (reg.regId);
}


/**
 * Translate imm index value from vc4 to v3d
 */
SmallImm encodeSmallImm(RegOrImm const &src_reg) {
  assert(src_reg.is_imm());

  Word w = decodeSmallLit(src_reg.imm().val);
  SmallImm ret(w.intVal);
  return ret;
}


std::unique_ptr<Location> loc_ptr(Register const &reg) {
  std::unique_ptr<Location> ret;
  ret.reset(new Register(reg));
  return ret;
}


std::unique_ptr<Location> loc_acc(RegId regId, int max_id) {
  assert(regId >= 0 && regId <= max_id);
  std::unique_ptr<Location> ret;

  switch(regId) {
    case 0: ret = loc_ptr(r0); break;
    case 1: ret = loc_ptr(r1); break;
    case 2: ret = loc_ptr(r2); break;
    case 3: ret = loc_ptr(r3); break;
    case 4: ret = loc_ptr(r4); break;
    case 5: ret = loc_ptr(r5); break;
  }

  assert(ret.get() != nullptr);
  return ret;
}


void check_unhandled_registers(Reg reg, bool do_src_regs) {
  if (do_src_regs) {
    switch (reg.tag) {
      case REG_B:
        debug_break("check_unhandled_registers(): Not expecting REG_B any more, examine");
      break;

    case SPECIAL:
      if (is_dma_only_register(reg)) {
        throw Exception("The code uses DMA source registers. These are not supported for v3d.");
      }

      switch (reg.regId) {
        case SPECIAL_UNIFORM:
        case SPECIAL_ELEM_NUM:
        case SPECIAL_QPU_NUM:
          assertq(false, "check_unhandled_registers(): Not expecting this SPECIAL regId, should be handled before call()", true);
        break;

        default: break;
      }
      break;

      default: break;
    }

    return;
  }

  // Do dst regs
  switch (reg.tag) {
    case REG_B:
      debug_break("encodeDestReg(): Not expecting REG_B any more, examine");
      break;
    case SPECIAL:
      if (is_dma_only_register(reg)) {
        throw Exception("The code uses DMA destination registers. These are not supported for v3d.");
      }
      break;

    default: break;
  }
}


/**
 *
 */
std::unique_ptr<Location> encodeSrcReg(Reg reg) {
  check_unhandled_registers(reg, true);

  bool is_none = false;
  std::unique_ptr<Location> ret;

  switch (reg.tag) {
    case REG_A:
      check_reg(reg);
      ret.reset(new RFAddress(to_waddr(reg)));
      break;
    case ACC:
      ret = loc_acc(reg.regId, 4);  // r5 not allowed here (?)
      break;
    case NONE:
      is_none = true;
      breakpoint  // Apparently never reached
      break;

    default:
      assertq(false, "V3DLib: unexpected reg-tag in encodeSrcReg()");
  }

  if (ret.get() == nullptr && !is_none) {
    assertq(false, "V3DLib: missing case in encodeSrcReg()", true);
  }

  return ret;
}


std::unique_ptr<Location> encodeDestReg(V3DLib::Instr const &src_instr) {
  assert(!src_instr.isUniformLoad());

  bool is_none = false;
  std::unique_ptr<Location> ret;

  Reg reg;
  if (src_instr.tag == ALU) {
    reg = src_instr.ALU.dest;
  } else {
    assert(src_instr.tag == LI);
    reg = src_instr.LI.dest;
  }

  check_unhandled_registers(reg, false);

  switch (reg.tag) {
    case REG_A:
      check_reg(reg);
      ret.reset(new RFAddress(to_waddr(reg)));
      break;

    case ACC:
      ret = loc_acc(reg.regId, 5);
      break;

    case SPECIAL:
      switch (reg.regId) {
        // These DMA registers *are* handled
        // They get translated to the corresponding v3d registers
        // TODO get VPM/DMA out of sight
        case SPECIAL_VPM_WRITE:           // Write TMU, to set data to write
          ret = loc_ptr(tmud);
          break;
        case SPECIAL_DMA_ST_ADDR:         // Write TMU, to set memory address to write to
          ret = loc_ptr(tmua);
          break;
        case SPECIAL_TMU0_S:              // Read TMU
          ret = loc_ptr(tmua);
          break;

        // SFU registers
        case SPECIAL_SFU_RECIP    : ret = loc_ptr(recip);     break;
        case SPECIAL_SFU_RECIPSQRT: ret = loc_ptr(rsqrt);     break;  // Alternative: register rsqrt2
        case SPECIAL_SFU_EXP      : ret = loc_ptr(exp);       break;
        case SPECIAL_SFU_LOG      : ret = loc_ptr(log);       break;

        default:
          assertq(false, "encodeDestReg(): not expecting reg tag", true);
          break;
      }
      break;
    case NONE: {
      // As far as I can tell, there is no such thing as a NONE register on v3d;
      // it may be one of the bits in `struct v3d_qpu_sig`.
      //
      // The first time I encountered this was in (V3DLib target code):
      //       _ <-{sf} or(B6, B6)
      //
      // The idea seems to be to set the CNZ flags depending on the value of a given rf-register.
      // So, for the time being, we will set a condition (how? Don't know for sure yet) if
      // srcA and srcB are the same in this respect, and set target same as both src's.
      is_none = true;
      assert(src_instr.setCond().flags_set());
      assert(src_instr.tag == ALU);

      auto &srcA = src_instr.ALU.srcA;

      // srcA and srcB are the same rf-register
      if (srcA.is_reg()
      && (srcA.reg().tag == REG_A || srcA.reg().tag == REG_B)
      && (srcA == src_instr.ALU.srcB)
      ) {
        ret = encodeSrcReg(srcA.reg());
      } else {
        breakpoint  // case not handled yet
      }
    }
    break;

    default:
      assertq(false, "V3DLib: unexpected reg tag in encodeDestReg()");
  }

  if (ret.get() == nullptr && !is_none) {
    fprintf(stderr, "V3DLib: missing case in encodeDestReg\n");
    assert(false);
  }

  return ret;
}


/**
 * For v3d, the QPU and ELEM num are not special registers but instructions.
 *
 * In order to not disturb the code translation too much, they are derived from the target instructions:
 *
 *    mov(ACC0, QPU_ID)   // vc4: QPU_NUM  or SPECIAL_QPU_NUM
 *    mov(ACC0, ELEM_ID)  // vc4: ELEM_NUM or SPECIAL_ELEM_NUM
 *
 * This is the **only** operation in which they can be used.
 * This function checks for proper usage.
 * These special cases get translated to `tidx(r0)` and `eidx(r0)` respectively, as a special case
 * for A_BOR.
 *
 * If the check fails, a fatal exception is thrown.
 *
 * ==================================================================================================
 *
 * * Both these instructions use r0 here; this might produce conflicts with other instructions
 *   I haven't found a decent way yet to compensate for this.
 */
void checkSpecialIndex(V3DLib::Instr const &src_instr) {
  if (src_instr.tag != ALU) {
    return;  // no problem here
  }

  auto srca = src_instr.ALU.srcA;
  auto srcb = src_instr.ALU.srcB;

  bool a_is_elem_num = (srca.is_reg() && srca.reg().tag == SPECIAL && srca.reg().regId == SPECIAL_ELEM_NUM);
  bool a_is_qpu_num  = (srca.is_reg() && srca.reg().tag == SPECIAL && srca.reg().regId == SPECIAL_QPU_NUM);
  bool b_is_elem_num = (srcb.is_reg() && srcb.reg().tag == SPECIAL && srcb.reg().regId == SPECIAL_ELEM_NUM);
  bool b_is_qpu_num  = (srcb.is_reg() && srcb.reg().tag == SPECIAL && srcb.reg().regId == SPECIAL_QPU_NUM);
  bool a_is_special  = a_is_elem_num || a_is_qpu_num;
  bool b_is_special  = b_is_elem_num || b_is_qpu_num;

  if (!a_is_special && !b_is_special) {
    return;  // Nothing to do
  }

  if (src_instr.ALU.op.value() != ALUOp::A_BOR) {
    // All other instructions disallowed
    fatal("For v3d, special registers QPU_NUM and ELEM_NUM can only be used in a move instruction");
    return;
  }

  assertq((a_is_special && b_is_special), "src a and src b must both be special for QPU and ELEM nums");
  assertq(srca == srcb, "checkSpecialIndex(): src a and b must be the same if they are both special num's");
}


/**
 * Pre: `checkSpecialIndex()` has been called
 */
bool is_special_index(V3DLib::Instr const &src_instr, Special index ) {
  assert(index == SPECIAL_ELEM_NUM || index == SPECIAL_QPU_NUM);

  if (src_instr.tag != ALU) {
    return false;
  }

  if (src_instr.ALU.op.value() != ALUOp::A_BOR) {
    return false;
  }

  auto srca = src_instr.ALU.srcA;
  auto srcb = src_instr.ALU.srcB;
  bool a_is_special = (srca.is_reg() && srca.reg().tag == SPECIAL && srca.reg().regId == index);
  bool b_is_special = (srcb.is_reg() && srcb.reg().tag == SPECIAL && srcb.reg().regId == index);

  return (a_is_special && b_is_special);
}


/**
 */
void setCondTag(AssignCond cond, v3d::Instr &out_instr) {
  if (cond.is_always()) {
    return;
  }
  assertq(cond.tag != AssignCond::Tag::NEVER, "Not expecting NEVER (yet)", true);
  assertq(cond.tag == AssignCond::Tag::FLAG,  "const.tag can only be FLAG here");  // The only remaining option

  // NOTE: condition tags are set for add alu only here
  // TODO: Set for mul tag as well if required
  //       Prob the easiest is to always set them for both for now

  switch(cond.flag) {
    case ZS:
    case NS:
      out_instr.ifa(); 
      break;
    case ZC:
    case NC:
      out_instr.ifna(); 
      break;
    default:  assert(false);
  }

}


void setCondTag(AssignCond cond, Instructions &ret) {
  for (auto &instr : ret) {
    setCondTag(cond, instr);
  }
}


void handle_condition_tags(V3DLib::Instr const &src_instr, Instructions &ret) {
  auto &cond = src_instr.ALU.cond;

  // src_instr.ALU.cond.tag has 3 possible values: NEVER, ALWAYS, FLAG
  assertq(cond.tag != AssignCond::Tag::NEVER, "NEVER encountered in ALU.cond.tag", true);          // Not expecting it
  assertq(cond.tag == AssignCond::Tag::FLAG || cond.is_always(), "Really expecting FLAG here", true); // Pedantry

  auto const &setCond = src_instr.setCond();

  if (setCond.flags_set()) {
    // Set a condition flag with current instruction
    assertq(cond.is_always(), "Currently expecting only ALWAYS here", true);

    // Note that the condition is only set for the last in the list.
    // Any preceding instructions are assumed to be for calculating the condition
    Instr &instr = ret.back();

    assertq(setCond.tag() == SetCond::Z || setCond.tag() == SetCond::N,
      "Unhandled setCond flag", true);

    if (setCond.tag() == SetCond::Z) {
      instr.pushz();
    } else {
      instr.pushn();
    }

  } else {
    // use flag as run condition for current instruction(s)
    if (cond.is_always()) {
      return; // ALWAYS executes always (duh, is default)
    }

    setCondTag(cond, ret);
  }
}


bool translateOpcode(V3DLib::Instr const &src_instr, Instructions &ret) {
  bool did_something = true;

  auto reg_a = src_instr.ALU.srcA;
  auto reg_b = src_instr.ALU.srcB;

  auto dst_reg = encodeDestReg(src_instr);

  if (dst_reg && reg_a.is_reg() && reg_b.is_reg()) {
    checkSpecialIndex(src_instr);
    if (is_special_index(src_instr, SPECIAL_QPU_NUM)) {
      ret << tidx(*dst_reg);
    } else if (is_special_index(src_instr, SPECIAL_ELEM_NUM)) {
      ret << eidx(*dst_reg);
    } else if (reg_a.reg().tag == NONE && reg_b.reg().tag == NONE) {
      assert(src_instr.ALU.op.noOperands());

      switch (src_instr.ALU.op.value()) {
        case ALUOp::A_TIDX:  ret << tidx(*dst_reg); break;
        case ALUOp::A_EIDX:  ret << eidx(*dst_reg); break;
        default:
          assertq("unimplemented op, input none", true);
          did_something = false;
        break;
      }
    } else if (reg_a.reg().tag != NONE && reg_b.reg().tag == NONE) {
      // 1 input
      auto src_a = encodeSrcReg(reg_a.reg());
      assert(src_a);

      switch (src_instr.ALU.op.value()) {
        case ALUOp::A_FFLOOR:  ret << ffloor(*dst_reg, *src_a); break;
        case ALUOp::A_FSIN:    ret << fsin(*dst_reg, *src_a);    break;
        default:
          assertq("unimplemented op, input reg", true);
          did_something = false;
        break;
      }
    } else {
      auto src_a = encodeSrcReg(reg_a.reg());
      auto src_b = encodeSrcReg(reg_b.reg());
      assert(src_a && src_b);

      switch (src_instr.ALU.op.value()) {
        case ALUOp::A_ASR:   ret << asr(*dst_reg, *src_a, *src_b);          break;
        case ALUOp::A_ADD:   ret << add(*dst_reg, *src_a, *src_b);          break;
        case ALUOp::A_SUB:   ret << sub(*dst_reg, *src_a, *src_b);          break;
        case ALUOp::A_BOR:   ret << bor(*dst_reg, *src_a, *src_b);          break;
        case ALUOp::A_BAND:  ret << band(*dst_reg, *src_a, *src_b);         break;
        case ALUOp::M_FMUL:  ret << nop().fmul(*dst_reg, *src_a, *src_b);   break;
        case ALUOp::M_MUL24: ret << nop().smul24(*dst_reg, *src_a, *src_b); break;
        case ALUOp::A_FSUB:  ret << fsub(*dst_reg, *src_a, *src_b);         break;
        case ALUOp::A_FADD:  ret << fadd(*dst_reg, *src_a, *src_b);         break;
        case ALUOp::A_MIN:   ret << min(*dst_reg, *src_a, *src_b);          break;
        case ALUOp::A_MAX:   ret << max(*dst_reg, *src_a, *src_b);          break;
        default:
          assertq("unimplemented op, input reg, reg", true);
          did_something = false;
        break;
      }
    }
  } else if (dst_reg && reg_a.is_reg() && reg_b.is_imm()) {
    auto src_a = encodeSrcReg(reg_a.reg());
    assert(src_a);
    SmallImm imm = encodeSmallImm(reg_b);

    switch (src_instr.ALU.op.value()) {
      case ALUOp::A_SHL:   ret << shl(*dst_reg, *src_a, imm);          break;
      case ALUOp::A_SHR:   ret << shr(*dst_reg, *src_a, imm);          break;
      case ALUOp::A_ASR:   ret << asr(*dst_reg, *src_a, imm);          break;
      case ALUOp::A_BAND:  ret << band(*dst_reg, *src_a, imm);         break;
      case ALUOp::A_SUB:   ret << sub(*dst_reg, *src_a, imm);          break;
      case ALUOp::A_ADD:   ret << add(*dst_reg, *src_a, imm);          break;
      case ALUOp::A_FADD:  ret << fadd(*dst_reg, *src_a, imm);         break;
      case ALUOp::A_FSUB:  ret << fsub(*dst_reg, *src_a, imm);         break;
      case ALUOp::M_FMUL:  ret << nop().fmul(*dst_reg, *src_a, imm);   break;
      case ALUOp::M_MUL24: ret << nop().smul24(*dst_reg, *src_a, imm); break;
      case ALUOp::A_ItoF:  ret << itof(*dst_reg, *src_a, imm);         break;
      case ALUOp::A_FtoI:  ret << ftoi(*dst_reg, *src_a, imm);         break;
      case ALUOp::A_BXOR:  ret << bxor(*dst_reg, *src_a, imm);         break;
      default:
        assertq("unimplemented op, input reg, imm", true);
        did_something = false;
      break;
    }
  } else if (dst_reg && reg_a.is_imm() && reg_b.is_reg()) {
    SmallImm imm = encodeSmallImm(reg_a);
    auto src_b   = encodeSrcReg(reg_b.reg());
    assert(src_b);

    switch (src_instr.ALU.op.value()) {
      case ALUOp::A_SHL:   ret << shl(*dst_reg, imm, *src_b);          break;
      case ALUOp::M_MUL24: ret << nop().smul24(*dst_reg, imm, *src_b); break;
      case ALUOp::M_FMUL:  ret << nop().fmul(*dst_reg, imm, *src_b);   break;
      case ALUOp::A_FSUB:  ret << fsub(*dst_reg, imm, *src_b);         break;
      case ALUOp::A_SUB:   ret << sub(*dst_reg, imm, *src_b);          break;
      case ALUOp::A_ADD:   ret << add(*dst_reg, imm, *src_b);          break;
      case ALUOp::A_FADD:  ret << fadd(*dst_reg, imm, *src_b);         break;
      default:
        assertq("unimplemented op, input imm, reg", true);
        did_something = false;
      break;
    }
  } else if (dst_reg && reg_a.is_imm() && reg_b.is_imm()) {
    SmallImm imm_a = encodeSmallImm(reg_a);
    SmallImm imm_b = encodeSmallImm(reg_b);

    switch (src_instr.ALU.op.value()) {
      case ALUOp::A_BOR:   ret << bor(*dst_reg, imm_a, imm_b);          break;
      default:
        assertq("unimplemented op, input imm, imm", true);
        did_something = false;
      break;
    }
  } else {
    assertq("Unhandled combination of inputs/output", true);
    did_something = false;
  }

  return did_something;
}


/**
 * @return true if rotate handled, false otherwise
 */
bool translateRotate(V3DLib::Instr const &instr, Instructions &ret) {
  if (!instr.ALU.op.isRot()) {
    return false;
  }

  // dest is location where r1 (result of rotate) must be stored 
  auto dst_reg = encodeDestReg(instr);
  assert(dst_reg);
  assertq(dst_reg->to_mux() != V3D_QPU_MUX_R1, "Rotate can not have destination register R1", true);

  auto reg_a = instr.ALU.srcA;
  auto src_a = encodeSrcReg(reg_a.reg());
  auto reg_b = instr.ALU.srcB;                  // reg b is either r5 or small imm

  if (src_a->to_mux() != V3D_QPU_MUX_R0) {
    ret << mov(r0, *src_a).comment("moving param 2 of rotate to r0. WARNING: r0 might already be in use, check!");
  }


  // TODO: the Target source step already adds a nop.
  //       With the addition of previous mov to r0, the 'other' nop becomes useless, remove that one for v3d.
  ret << nop().comment("NOP required for rotate");

  if (reg_b.is_reg()) {
    breakpoint

    assert(instr.ALU.srcB.reg().tag == ACC && instr.ALU.srcB.reg().regId == 5);  // reg b must be r5
    auto src_b = encodeSrcReg(reg_b.reg());

    ret << rotate(r1, r0, *src_b);

  } else {
    SmallImm imm = encodeSmallImm(reg_b);  // Legal values small imm tested in rotate()
    ret << rotate(r1, r0, imm);
  }

  ret << bor(*dst_reg, r1, r1);

  return true;
}


/**
 * Convert powers of 2 of direct small immediates
 */
bool convert_int_powers(Instructions &output, int in_value) {
  if (in_value < 0)  return false;  // only positive values for now
  if (in_value < 16) return false;  // don't bother with values within range

  int value = in_value;
  int left_shift = 0;

  while (value != 0 && (value & 1) == 0) {
    left_shift++;
    value >>= 1;
  }

  if (left_shift == 0) return false;

  int rep_value;
  if (!SmallImm::int_to_opcode_value(value, rep_value)) return false;

  SmallImm imm(rep_value);

  std::string cmt;
  cmt << "Load immediate " << in_value;

  Instructions ret;
  ret << mov(r0, imm).comment(cmt);
  ret << shl(r0, r0, SmallImm(left_shift));

  output << ret;
  return true;
}


/**
 * Blunt tool for converting all int's.
 *
 * **NOTE:** uses r0, r1 and r2 internally, register conflict possible
 *
 * @param output    output parameter, sequence of instructions to
 *                  add generated code to
 * @param in_value  value to encode
 *
 * @return  true if conversion successful, false otherwise
 */
bool encode_int_immediate(Instructions &output, int in_value) {
  Instructions ret;
  uint32_t value = (uint32_t) in_value;
  uint32_t nibbles[8];  // was 7 (idiot)

  for (int i = 0; i < 8; ++i) {
    nibbles[i] = (value  >> (4*i)) & 0xf;
  }

  bool did_first = false;
  for (int i = 7; i >= 0; --i) {
    if (nibbles[i] == 0) continue;

    SmallImm imm(nibbles[i]);

    // r0 is used as temp value,
    // result is in r1

    if (!did_first) {
      ret << mov(r1, imm);  // TODO Gives segfault on p4-3 (arm32)
                            //      No clue why, works fine on silke (arm64) and pi3

      if (i > 0) {
        if (convert_int_powers(ret, 4*i)) {
          // r0 now contains value for left shift
          ret << shl(r1, r1, r0);
        } else {
          ret << shl(r1, r1, SmallImm(4*i));
        }
      }
      did_first = true;
    } else {
      if (i > 0) {
        if (convert_int_powers(ret, 4*i)) {
          // r0 now contains value for left shift
          //ret << mov(r2, imm);
          //ret << shl(r0, r2, r0);
          ret << shl(r0, imm, r0);
        } else {
          ret << mov(r0, imm);
          ret << shl(r0, r0, SmallImm(4*i));
        }

        ret << bor(r1, r1, r0);
      } else {
        ret << bor(r1, r1, imm);
      }
    }
  }

  if (ret.empty()) return false;  // Not expected, but you never know

  std::string cmt;
  cmt << "Load immediate " << in_value;
  ret.front().comment(cmt);

  std::string cmt2;
  cmt2 << "End load immediate " << in_value;
  ret.back().comment(cmt2);

  output << ret;
  return true;
}


bool encode_int(Instructions &ret, std::unique_ptr<Location> &dst, int value) {
  bool success = true;
  int rep_value;

  if (SmallImm::int_to_opcode_value(value, rep_value)) {  // direct translation
    SmallImm imm(rep_value);
    ret << mov(*dst, imm);
  } else if (convert_int_powers(ret, value)) {            // powers of 2 of basic small int's 
    ret << mov(*dst, r0);
  } else if (encode_int_immediate(ret, value)) {          // Use full blunt conversion (heavy but always works)
    ret << mov(*dst, r1);
  } else {
    success = false;                                      // Conversion failed
  }

  return success;
}


bool encode_float(Instructions &ret, std::unique_ptr<Location> &dst, float value) {
  bool success = true;
  int rep_value;

  if (value < 0 && SmallImm::float_to_opcode_value(-value, rep_value)) {
    ret << nop().fmov(*dst, rep_value)
        << fsub(*dst, 0, *dst);                   // Works because float zero is 0x0
  } else if (SmallImm::float_to_opcode_value(value, rep_value)) {
    ret << nop().fmov(*dst, rep_value);
  } else if ((value == (float) ((int) value))) {  // Special case: float is encoded int, no fraction
    int int_value = (int) value;
    SmallImm dummy(0);                            // TODO why need this???

    if (encode_int(ret, dst, int_value)) {
      ret  << itof(*dst, *dst, dummy);
    } else {
      assertq("Full-int float conversion failed", true);
      success = false;
    }
  } else {
    // Do the full blunt int conversion
    int int_value = *((int *) &value);
    if (encode_int_immediate(ret, int_value)) {
      ret << mov(*dst, r1);                       // Result is int but will be handled as float downstream
    } else {
      success = false;
    }
  }

  return success;
}


Instructions encodeLoadImmediate(V3DLib::Instr const full_instr) {
  assert(full_instr.tag == LI);
  auto &instr = full_instr.LI;
  auto dst = encodeDestReg(full_instr);

  Instructions ret;

  std::string err_label;
  std::string err_value;

  switch (instr.imm.tag()) {
  case Imm::IMM_INT32: {
    int value = instr.imm.intVal();

    if (!encode_int(ret, dst, value)) {
      // Conversion failed, output error
      err_label = "int";
      err_value = std::to_string(value);
    }
  }
  break;

  case Imm::IMM_FLOAT32: {
    float value = instr.imm.floatVal();

    if (!encode_float(ret, dst, value)) {
      // Conversion failed, output error
      err_label = "float";
      err_value = std::to_string(value);
    }
  }
  break;

  case Imm::IMM_MASK:
    debug_break("encodeLoadImmediate(): IMM_MASK not handled");
  break;

  default:
    debug_break("encodeLoadImmediate(): unknown tag value");
  break;
  }

  if (!err_value.empty()) {
    assert(!err_label.empty());

    std::string str = "LI: Can't handle ";
    str += err_label + " value '" + err_value;
    str += "' as small immediate";

    breakpoint
    local_errors << str;
    ret << nop();
    ret.back().comment(str);
  }


  if (full_instr.setCond().flags_set()) {
    breakpoint;  // to check what flags need to be set - case not handled yet
  }

  setCondTag(instr.cond, ret);
  return ret;
}


Instructions encodeALUOp(V3DLib::Instr instr) {
  Instructions ret;

  if (instr.isUniformLoad()) {
    Reg dst_reg = instr.ALU.dest;
    uint8_t rf_addr = to_waddr(dst_reg);
    ret << nop().ldunifrf(rf(rf_addr));
   } else if (translateRotate(instr, ret)) {
    handle_condition_tags(instr, ret);
  } else if (translateOpcode(instr, ret)) {
    handle_condition_tags(instr, ret);
  } else {
    assertq(false, "Missing translate operation for ALU instruction", true);  // Something missing, check
  }

  assert(!ret.empty());
  return ret;
}


/**
 * Convert conditions from Target source to v3d
 *
 * Incoming conditions are vc4 only, the conditions don't exist on v3d.
 * They therefore need to be translated.
 */
void encodeBranchCondition(v3d::instr::Instr &dst_instr, V3DLib::BranchCond src_cond) {
  // TODO How to deal with:
  //
  //      dst_instr.na0();
  //      dst_instr.a0();

  if (src_cond.tag == COND_ALWAYS) {
    return;  // nothing to do
  } else if (src_cond.tag == COND_ALL) {
    switch (src_cond.flag) {
      case ZC:
      case NC:
        dst_instr.allna();
        break;
      case ZS:
      case NS:
        dst_instr.alla();
        break;
      default:
        debug_break("Unknown branch condition under COND_ALL");  // Warn me if this happens
    }
  } else if (src_cond.tag == COND_ANY) {
    switch (src_cond.flag) {
      case ZC:
      case NC:
        dst_instr.anyna();  // TODO: verify
        break;
      case ZS:
      case NS:
        dst_instr.anya();  // TODO: verify
        break;
      default:
        debug_break("Unknown branch condition under COND_ANY");  // Warn me if this happens
    }
  } else {
    debug_break("Branch condition not COND_ALL or COND_ANY");  // Warn me if this happens
  }
}


/**
 * Create a branch instruction, including any branch conditions,
 * from Target source instruction.
 */
v3d::instr::Instr encodeBranchLabel(V3DLib::Instr src_instr) {
  assert(src_instr.tag == BRL);
  auto &instr = src_instr.BRL;

  // Prepare as branch without offset but with label
  auto dst_instr = branch(0, true);
  dst_instr.label(instr.label);
  encodeBranchCondition(dst_instr, instr.cond);

  return dst_instr;
}


/**
 * Convert intermediate instruction into core instruction.
 *
 * **Pre:** All instructions not meant for v3d are detected beforehand and flagged as error.
 */
Instructions encodeInstr(V3DLib::Instr instr) {
  Instructions ret;

  // Encode core instruction
  switch (instr.tag) {
    //
    // Unhandled tags - ignored or should have been handled beforehand
    //
    case BR:
      assertq(false, "Not expecting BR any more, branch creation now goes with BRL", true);
    break;

    case INIT_BEGIN:
    case INIT_END:
    case END:         // vc4 end program marker
      assertq(false, "Not expecting INIT or END tag here", true);
    break;

    //
    // Label handling
    //
    case LAB: {
      // create a label meta-instruction
      Instr n;
      n.is_label(true);
      n.label(instr.label());
      
      ret << n;
    }
    break;

    case BRL: {
      // Create branch instruction with label
      ret << encodeBranchLabel(instr);
    }
    break;

    //
    // Handled tags
    //
    case LI:           ret << encodeLoadImmediate(instr); break;
    case ALU:          ret << encodeALUOp(instr);         break;
    case TMU0_TO_ACC4: ret << nop().ldtmu(r4);            break;
    case NO_OP:        ret << nop();                      break;
    case TMUWT:        ret << tmuwt();                    break;

    default:
      fatal("v3d: missing case in encodeInstr");
  }

  assert(!ret.empty());

  if (!ret.empty()) {
    ret.front().transfer_comments(instr);
  }

  return ret;
}


/**
 * This is where standard initialization code can be added.
 *
 * Called;
 * - after code for loading uniforms has been encoded
 * - any other target initialization code has been added
 * - before the encoding of the main body.
 *
 * Serious consideration: Any regfile registers used in the generated code here,
 * have not participated in the liveness determination. This may lead to screwed up variable assignments.
 * **Keep this in mind!**
 */
Instructions encode_init() {
  Instructions ret;
  ret << instr::enable_tmu_read();

  return ret;
}


#ifdef DEBUG

/**
 * Check assumption: uniform loads are always at the top of the instruction list.
 */
bool checkUniformAtTop(V3DLib::Instr::List const &instrs) {
  bool doing_top = true;

  for (int i = 0; i < instrs.size(); i++) {
    V3DLib::Instr instr = instrs[i];
    if (doing_top) {
      if (instr.isUniformLoad()) {
        continue;  // as expected
      }

      doing_top = false;
    } else {
      if (!instr.isUniformLoad()) {
        continue;  // as expected
      }

      return false;  // Encountered uniform NOT at the top of the instruction list
    }
  }

  return true;
}

#endif  // DEBUG


/**
 * Criteria are intentional extremely strict.
 * These will be relaxed when further cases for optimization are encountered.
 *
 * Note that index can change!
 */
bool handle_target_specials(Instructions &ret, V3DLib::Instr::List const &instrs, int &index) {
  if (index + 1 == instrs.size()) return false;

  auto const &instr = instrs[index];
  auto const &next_instr = instrs[index + 1];
  if (instr.isCondAssign()) return false;
  if (next_instr.isCondAssign()) return false;

  //
  // If possible, combine a TMU gather with the next statement
  // Currently, only add as next statement allowed
  //
  if (instr.isTMUAWrite(true)) {
    bool simple_int_add = (next_instr.tag == ALU && next_instr.ALU.op == ALUOp::A_ADD);
    if (!simple_int_add) return false; 

    // Can combine if there are at most two different source values
    int unique_src_count = 1;  // for instr src A

    // Don't feel like making this pretty right now
    if (instr.ALU.srcA != instr.ALU.srcB) ++unique_src_count;
    if (instr.ALU.srcA != next_instr.ALU.srcA && instr.ALU.srcB != next_instr.ALU.srcA) ++unique_src_count;

    if (instr.ALU.srcA      != next_instr.ALU.srcB
     && instr.ALU.srcB      != next_instr.ALU.srcB
     && next_instr.ALU.srcA != next_instr.ALU.srcB) ++unique_src_count;

    if (unique_src_count > 2) {
      std::string msg = "unique_src_count: ";
      msg << unique_src_count                      << "\n"
          << "  instr     : " << instr.dump()      << "\n"
          << "  next_instr: " << next_instr.dump() << "\n";

      assertq(false, msg); // Warn me if this happens, will need unit test
      return false;
    }

    //
    // Can combine!
    //
    //std::cout << "Target instr is TMU fetch: " << instr.dump() << std::endl;
    //std::cout << "Next target instr is add: " << next_instr.dump() << std::endl;

    Instructions tmp;
    assertq(translateOpcode(instr, tmp), "translateOpcode() failed");
    assert(tmp.size() == 1);

    auto dst   = encodeDestReg(next_instr);
    assert(dst);
    auto reg_a = next_instr.ALU.srcA;
    auto reg_b = next_instr.ALU.srcB;
    auto src_a = encodeSrcReg(reg_a.reg());
    auto src_b = encodeSrcReg(reg_b.reg());
    assert(src_a && src_b);
    tmp[0].add(*dst, *src_a, *src_b);
    tmp[0].comment(instr.comment());
    tmp[0].comment(next_instr.comment());

    //std::cout << "result: " << tmp[0].mnemonic(true) << std::endl;
    index++;
    ret << tmp;
    return true;
  }

  return false;
}


/**
 * Translate instructions from target to v3d
 */
void _encode(V3DLib::Instr::List const &instrs, Instructions &instructions) {
  assert(checkUniformAtTop(instrs));
  bool prev_was_init_begin = false;
  bool prev_was_init_end    = false;

  // Main loop
  for (int i = 0; i < instrs.size(); i++) {
    V3DLib::Instr instr = instrs[i];
    assertq(!instr.isZero(), "Zero instruction encountered", true);
    check_instruction_tag_for_platform(instr.tag, false);

    if (instr.tag == INIT_BEGIN) {
      prev_was_init_begin = true;
    } else if (instr.tag == INIT_END) {
      instructions << encode_init();
      prev_was_init_end = true;
    } else {
      Instructions ret;

      if (!handle_target_specials(ret, instrs, i)) {
        ret = v3d::encodeInstr(instr);
      }

      if (prev_was_init_begin) {
        ret.front().header("Init block");
        prev_was_init_begin = false;
      }

      if (prev_was_init_end) {
        ret.front().header("Main program");
        prev_was_init_end = false;
      }

      instructions << ret;
    }
  }

  instructions << sync_tmu()
               << end_program();
}


using Code       = SharedArray<uint64_t>;
using UniformArr = SharedArray<uint32_t>;


void invoke(int numQPUs, Code &codeMem, int qpuCodeMemOffset, IntList &params) {
#ifndef QPU_MODE
  assertq(false, "Cannot run v3d invoke(), QPU_MODE not enabled");
#else
  assert(codeMem.size() != 0);

  UniformArr unif(params.size() + 3);
  UniformArr done(1);
  done[0] = 0;

  // The first two slots in uniforms for vc4 are used for qpu number and num qpu's respectively
  // We do the same for v3d, so as not to screw up the logic too much.
  int offset = 0;

  unif[offset++] = 0;        // qpu number (id for current qpu) - 0 is for 1 QPU
  unif[offset++] = numQPUs;  // num qpu's running for this job

  for (int j = 0; j < params.size(); j++) {
    unif[offset++] = params[j];
  }

  // The last item is for the 'done' location;
  // Not sure if this is the correct slot to put it
  // TODO: scrutinize the python project for this
  unif[offset] = (uint32_t) done.getAddress();

  Driver drv;
  drv.add_bo(getBufferObject().getHandle());
  drv.execute(codeMem, &unif, numQPUs);
#endif  // QPU_MODE
}


}  // anon namespace


///////////////////////////////////////////////////////////////////////////////
// Class KernelDriver
///////////////////////////////////////////////////////////////////////////////

KernelDriver::KernelDriver() : V3DLib::KernelDriver(V3dBuffer), qpuCodeMem(code_bo)  {}


void KernelDriver::encode() {
  if (instructions.size() > 0) return;  // Don't bother if already encoded
  if (has_errors()) return;              // Don't do this if compile errors occured
  assert(!qpuCodeMem.allocated());

  // Encode target instructions
  _encode(m_targetCode, instructions);
  removeLabels(instructions);

  if (!local_errors.empty()) {
    breakpoint
  }

  errors << local_errors;
  local_errors.clear();
}


/**
 * Generate the opcodes for the currrent v3d instruction sequence
 *
 * The translation/removal of labels happens here somewhere 
 */
std::vector<uint64_t> KernelDriver::to_opcodes() {
  assert(instructions.size() > 0);

  std::vector<uint64_t> code;  // opcodes for v3d

  for (auto const &inst : instructions) {
    code << inst.code();
  }

  return code;
}


void KernelDriver::compile_intern() {
  Timer t1("compile_intern", true);

  obtain_ast();

  Timer t3("translate_stmt");
  translate_stmt(m_targetCode, m_body);  // performance hog 2 12/45s
  t3.end();

  insertInitBlock(m_targetCode);
  add_init(m_targetCode);

  Timer t5("compile_postprocess");
  compile_postprocess(m_targetCode);  // performance hog 1 31/45s
  t5.end();

  encode();
}


void KernelDriver::allocate() {
  assert(!instructions.empty());

  // Assumption: code in a kernel, once allocated, doesn't change
  if (qpuCodeMem.allocated()) {
    assert(instructions.size() >= qpuCodeMem.size());  // Tentative check, not perfect
                                                       // actual opcode seq can be smaller due to removal labels
  } else {
    std::vector<uint64_t> code = to_opcodes();
    assert(!code.empty());

    // Allocate memory for the QPU code
    uint32_t size_in_bytes = (uint32_t) (sizeof(uint64_t)*code.size());
    code_bo.alloc(size_in_bytes);
    qpuCodeMem.alloc((uint32_t) code.size());
    qpuCodeMem.copyFrom(code);  // Copy kernel to code memory

    qpuCodeMemOffset = (int) size_in_bytes;
  }
}


void KernelDriver::invoke_intern(int numQPUs, IntList &params) {
  if (numQPUs != 1 && numQPUs != 8) {
    error("Num QPU's must be 1 or 8", true);
  }

  assertq(!has_errors(), "v3d kernels has errors, can not invoke");

  allocate();
  assert(qpuCodeMem.allocated());

  // Allocate memory for the parameters if not done already
  // TODO Not used in v3d, do we need this?
  unsigned numWords = (12*MAX_KERNEL_PARAMS + 12*2);
  if (paramMem.allocated()) {
    assert(paramMem.size() == numWords);
  } else {
    paramMem.alloc(numWords);
  }

  //
  // NOTE: it doesn't appear to be necessary to add the BO for the code to the
  //       used BO list in Driver (used in next call). All unit tests pass without
  //       calling Driver::add_bo() in next call.
  //       This is something to keep in mind; it might go awkwards later on.
  //
  v3d::invoke(numQPUs, qpuCodeMem, qpuCodeMemOffset, params);
}


void KernelDriver::emit_opcodes(FILE *f) {
  fprintf(f, "Opcodes for v3d\n");
  fprintf(f, "===============\n\n");

  if (instructions.empty()) {
    fprintf(f, "<No opcodes to print>\n");
  } else {
    for (auto const &instr : instructions) {
      fprintf(f, "%s\n", instr.mnemonic(true).c_str());
    }
  }

  fprintf(f, "\n");
  fflush(f);
}

}  // namespace v3d
}  // namespace V3DLib
