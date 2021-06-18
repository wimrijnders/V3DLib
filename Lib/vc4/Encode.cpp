#include "Encode.h"
#include <stdio.h>
#include <stdlib.h>
#include "Support/basics.h"  // fatal()
#include "Target/Satisfy.h"
#include "Target/Pretty.h"

namespace V3DLib {
namespace vc4 {
namespace {

class vc4_Instr {
public:
  enum Tag {
    NOP,
    ROT,
    ALU,
    LI,
    BR,
    END,
    SINC,
    SDEC,

    TMU0_TO_ACC4  // TODO RIP
  };

  uint32_t cond_add = 0;    // reused for LI, BR
  uint32_t cond_mul = 0;
  uint32_t waddr_add = 39;  // reused for LI
  uint32_t waddr_mul = 39;

  uint32_t addOp  = 0;
  uint32_t mulOp  = 0;
  uint32_t muxa   = 0;
  uint32_t muxb   = 0;
  uint32_t raddra = 39;
  uint32_t raddrb = 0;

  uint32_t li_imm = 0;  // Also used as BR target
  uint32_t sema_id = 0;

  void tag(Tag in_tag, bool imm = false) {
    m_tag = in_tag;

    switch(m_tag) {
      case NOP:  // default
      case LI:
        break;  // as is

      case ROT:
        m_sig = 13;
        break;

      case ALU:
        m_sig = imm?13:1;
        break;

      case BR:
        m_sig = 15;
        break;

      case END:
        m_sig = 3;
        raddrb = 39;
        break;

      case TMU0_TO_ACC4:
        m_sig = 10;
        raddrb = 39;
        break;

      case SINC:
      case SDEC:
        m_sig = 0xe8;  // NOTE: not a single nibble like the rest
        break;
    };
  }


  void sf(bool val) { assert(m_tag != BR); m_sf = val; }
  void ws(bool val) { assert(m_tag != BR); m_ws = val; }
  void rel(bool val) { assert(m_tag == BR); m_rel = val; }


  uint32_t high() const {
    uint32_t ret = (m_sig << 28) | (waddr_add << 6) | waddr_mul;

    if (m_tag == BR) {
      ret |= (cond_add << 20)
          |  (m_rel?(1 << 19):0);
    } else if (m_tag == SINC || m_tag == SDEC) {
      // NOTE sig shift different from rest, because sig not a nibble
      ret = (m_sig << 24) | (waddr_add << 6) | waddr_mul;
    } else if (m_tag != END && m_tag != TMU0_TO_ACC4) {
      ret |= (cond_add << 17) | (cond_mul << 14) 
          |  (m_sf?(1 << 13):0)
          | (m_ws?(1 << 12):0);
    }

    return ret;
  }


  uint32_t low() const {
    switch (m_tag) {
      case NOP:
        return  0;
      case ROT:
        return (mulOp << 29) | (raddra << 18) | (raddrb << 12);
      case ALU:
        return (mulOp << 29) | (addOp << 24) | (raddra << 18) | (raddrb << 12)
                     | (muxa << 9) | (muxb << 6)
                     | (muxa << 3) | muxb;
      case END:
      case TMU0_TO_ACC4:
        return (raddra << 18) | (raddrb << 12);
      case LI:
      case BR:
        return li_imm;
      case SINC:
        return sema_id;
      case SDEC:
        return (1 << 4) | sema_id;
    };

    assert(false);
    return 0;
  }

private:
  Tag m_tag = NOP;
  uint32_t m_sig = 14;        // 0xe0000000

  bool m_ws  = false;
  bool m_sf  = false;
  bool m_rel = false;
};

// ===============
// Condition flags
// ===============

uint32_t encodeAssignCond(AssignCond cond) {
  switch (cond.tag) {
    case AssignCond::Tag::NEVER:  return 0;
    case AssignCond::Tag::ALWAYS: return 1;
    case AssignCond::Tag::FLAG:
      switch (cond.flag) {
        case ZS: return 2;
        case ZC: return 3;
        case NS: return 4;
        case NC: return 5;
     }

    default:
      fatal("V3DLib: missing case in encodeAssignCond");
      return 0;
  }
}


// =================
// Branch conditions
// =================

uint32_t encodeBranchCond(BranchCond cond) {
  switch (cond.tag) {
    case COND_NEVER:
      fatal("V3DLib: 'never' condition not supported");
    case COND_ALWAYS: return 15;
    case COND_ALL:
      switch (cond.flag) {
        case ZS: return 0;
        case ZC: return 1;
        case NS: return 4;
        case NC: return 5;
        default: break;
      }
    case COND_ANY:
      switch (cond.flag) {
        case ZS: return 2;
        case ZC: return 3;
        case NS: return 6;
        case NC: return 7;
        default: break;
      }

    default:
      fatal("V3DLib: missing case in encodeBranchCond");
      return 0;
  }
}


// ================
// Register encoder
// ================

/**
 * @brief Determine the regfile and index combination to use for writes, for the passed 
 *        register definition 'reg'.
 *
 * This function deals exclusively with write values of the regfile registers.
 *
 * See also NOTES in header comment for `encodeSrcReg()`.
 *
 * @param reg   register definition for which to determine output value
 * @param file  out-parameter; the regfile to use (either REG_A or REG_B)
 *
 * @return index into regfile (A, B or both) of the passed register
 *
 * ----------------------------------------------------------------------------------------------
 * ## NOTES
 *
 * * The regfile location for `ACC4` is called `TMP_NOSWAP` in the doc. This is because
 *   special register `r4` (== ACC4) is read-only.
 *   TODO: check code if ACC4 is actually used. Better to name it TMP_NOSWAP
 *
 * * ACC5 has parentheses with extra function descriptions.
 *   This implies that the handling of ACC5 differs from the others (at least, for ACC[0123])
 *   TODO: Check code for this; is there special handling of ACC5? 
 */
uint32_t encodeDestReg(Reg reg, RegTag* file) {
  // Selection of regfile for the cases where using A or B doesn't matter
  RegTag AorB = REG_A;  // Select A as default
  if (reg.tag == REG_A || reg.tag == REG_B) {
    AorB = reg.tag;     // If the regfile is preselected in `reg`, use that.
  }

  switch (reg.tag) {
    case REG_A:
      assert(reg.regId >= 0 && reg.regId < 32);
      *file = REG_A; return reg.regId;
    case REG_B:
      assert(reg.regId >= 0 && reg.regId < 32);
      *file = REG_B; return reg.regId;
    case ACC:
      // See NOTES in header comment
      assert(reg.regId >= 0 && reg.regId <= 5); // !! ACC4 is TMP_NOSWAP, *not* r4
      *file = reg.regId == 5 ? REG_B : AorB; 
      return 32 + reg.regId;
    case SPECIAL:
      switch (reg.regId) {
        case SPECIAL_RD_SETUP:      *file = REG_A; return 49;
        case SPECIAL_WR_SETUP:      *file = REG_B; return 49;
        case SPECIAL_DMA_LD_ADDR:   *file = REG_A; return 50;
        case SPECIAL_DMA_ST_ADDR:   *file = REG_B; return 50;
        case SPECIAL_VPM_WRITE:     *file = AorB;  return 48;
        case SPECIAL_HOST_INT:      *file = AorB;  return 38;
        case SPECIAL_TMU0_S:        *file = AorB;  return 56;
        case SPECIAL_SFU_RECIP:     *file = AorB;  return 52;
        case SPECIAL_SFU_RECIPSQRT: *file = AorB;  return 53;
        case SPECIAL_SFU_EXP:       *file = AorB;  return 54;
        case SPECIAL_SFU_LOG:       *file = AorB;  return 55;
        default:                    break;
      }
    case NONE:
      // NONE maps to 'NOP' in regfile.
      *file = AorB; return 39;

    default:
      fatal("V3DLib: missing case in encodeDestReg");
      return 0;
  }
}


/**
 * @brief Determine regfile index and the read field encoding for alu-instructions, for the passed 
 *        register 'reg'.
 *
 * The read field encoding, output parameter `mux` is a bitfield in instructions `alu` and 
 * 'alu small imm'. It specifies the register(s) to use as input.
 *
 * This function deals exclusively with 'read' values.
 *
 * @param reg   register definition for which to determine output value
 * @param file  regfile to use. This is used mainly to validate the `regid` field in param `reg`. In specific
 *              cases where both regfile A and B are valid (e.g. NONE), it is used to select the regfile.
 * @param mux   out-parameter; value in ALU instruction encoding for fields `add_a`, `add_b`, `mul_a` and `mul_b`.
 *
 * @return index into regfile (A, B or both) of the passed register.
 *
 * ----------------------------------------------------------------------------------------------
 * ## NOTES
 *
 * * There are four combinations of access to regfiles:
 *   - read A
 *   - read B
 *   - write A
 *   - write B
 *
 * This is significant, because SPECIAL registers may only be accessible through a specific combination
 * of A/B and read/write.
 *
 * * References in VideoCore IV Reference document:
 *
 *   - Fields `add_a`, `add_b`, `mul_a` and `mul_b`: "Figure 4: ALU Instruction Encoding", page 26
 *   - mux value: "Table 3: ALU Input Mux Encoding", page 28
 *   - Index regfile: "Table 14: 'QPU Register Addess Map'", page 37.
 *
 * ----------------------------------------------------------------------------------------------
 * ## TODO
 *
 * * NONE/NOP - There are four distinct versions for `NOP`, A/B and no read/no write.
 *              Verify if those distinctions are important at least for A/B.
 *              They might be the same thing.
 */
uint32_t encodeSrcReg(Reg reg, RegTag file, uint32_t* mux) {
  assert (file == REG_A || file == REG_B);

  const uint32_t NO_REGFILE_INDEX = 0;  // Return value to use when there is no regfile mapping for the register

  // Selection of regfile for the cases that both A and B are possible.
  // Note that param `file` has precedence here.
  uint32_t AorB = (file == REG_A)? 6 : 7;

  switch (reg.tag) {
    case REG_A:
      assert(reg.regId >= 0 && reg.regId < 32 && file == REG_A);
      *mux = 6; return reg.regId;
    case REG_B:
      assert(reg.regId >= 0 && reg.regId < 32 && file == REG_B);
      *mux = 7; return reg.regId;
    case ACC:
      // ACC does not map onto a regfile for 'read'
      assert(reg.regId >= 0 && reg.regId <= 4);  // TODO index 5 missing, is this correct??
      *mux = reg.regId; return NO_REGFILE_INDEX;
    case NONE:
      // NONE maps to `NOP` in the regfile
      *mux = AorB; return 39;
    case SPECIAL:
      switch (reg.regId) {
        case SPECIAL_UNIFORM:     *mux = AorB;                     return 32;
        case SPECIAL_ELEM_NUM:    assert(file == REG_A); *mux = 6; return 38;
        case SPECIAL_QPU_NUM:     assert(file == REG_B); *mux = 7; return 38;
        case SPECIAL_VPM_READ:    *mux = AorB;                     return 48;
        case SPECIAL_DMA_LD_WAIT: assert(file == REG_A); *mux = 6; return 50;
        case SPECIAL_DMA_ST_WAIT: assert(file == REG_B); *mux = 7; return 50;
      }

    default:
      fatal("V3DLib: missing case in encodeSrcReg");
      return 0;
  }
}


// ===================
// Instruction encoder
// ===================

void encodeInstr(Instr instr, uint32_t* high, uint32_t* low) {
  vc4_Instr vc4_instr;

  // Convert intermediate instruction into core instruction
  switch (instr.tag) {
    case IRQ:
      instr.tag           = LI;
      instr.LI.m_setCond.clear();
      instr.LI.cond.tag   = AssignCond::Tag::ALWAYS;
      instr.LI.dest.tag   = SPECIAL;
      instr.LI.dest.regId = SPECIAL_HOST_INT;
      instr.LI.imm        = Imm(1);
      break;

    case DMA_LOAD_WAIT:
    case DMA_STORE_WAIT: {
      RegId src = instr.tag == DMA_LOAD_WAIT ? SPECIAL_DMA_LD_WAIT : SPECIAL_DMA_ST_WAIT;

      instr.tag                   = ALU;
      instr.ALU.m_setCond.clear();
      instr.ALU.cond.tag          = AssignCond::Tag::NEVER;
      instr.ALU.op                = ALUOp(ALUOp::A_BOR);
      instr.ALU.dest.tag          = NONE;

      instr.ALU.srcA.set_reg(SPECIAL, src);  // srcA is same as srcB
      instr.ALU.srcB.set_reg(SPECIAL, src);
      break;
    }

    default:
      break;  // rest passes through
  }

  // Encode core instruction
  switch (instr.tag) {
    // Not expecting these any more after previous step
    case IRQ:
    case DMA_LOAD_WAIT:
    case DMA_STORE_WAIT:
      assertq(false, "encodeInstr(): intermediate instruction can not be handled as core instruction");
      return;

    // Load immediate
    case LI: {
      auto &li = instr.LI;

      RegTag file;
      uint32_t cond = encodeAssignCond(li.cond) << 17;
      uint32_t waddr_add = encodeDestReg(li.dest, &file) << 6;
      uint32_t waddr_mul = 39;
      uint32_t ws   = (file == REG_A ? 0 : 1) << 12;
      uint32_t sf   = (li.m_setCond.flags_set()? 1 : 0) << 13;
      //*high         = 0xe0000000 | cond | ws | sf | waddr_add | waddr_mul;
      //*low          = li.imm.encode();

      ///////////////////
      debug("LI!");
      vc4_instr.tag(vc4_Instr::LI);
      vc4_instr.cond_add = encodeAssignCond(li.cond);
      vc4_instr.waddr_add = encodeDestReg(li.dest, &file);
      vc4_instr.sf(li.m_setCond.flags_set());
      vc4_instr.ws(file != REG_A);
      vc4_instr.li_imm = li.imm.encode();
      ///////////////////
    }
    break;

    case BR:  // Branch
      assertq(!instr.BR.target.useRegOffset, "Register offset not yet supported");

      vc4_instr.tag(vc4_Instr::BR);
      vc4_instr.cond_add = encodeBranchCond(instr.BR.cond);
      vc4_instr.rel(instr.BR.target.relative);
      vc4_instr.li_imm = 8*instr.BR.target.immOffset;
    break;

    case ALU: {
      auto &alu = instr.ALU;

      RegTag file;
      uint32_t dest  = encodeDestReg(alu.dest, &file);

      ///////////////////

      if (alu.op.isMul()) {
        vc4_instr.cond_mul  = encodeAssignCond(alu.cond);
        vc4_instr.waddr_mul = dest;
        vc4_instr.ws(file != REG_B);
      } else {
        vc4_instr.cond_add  = encodeAssignCond(alu.cond);
        vc4_instr.waddr_add = dest;
        vc4_instr.ws(file != REG_A);
      }

      vc4_instr.sf(alu.m_setCond.flags_set());
      ///////////////////


      if (alu.op.isRot()) {
        assert(alu.srcA.is_reg() && alu.srcA.reg().tag == ACC && alu.srcA.reg().regId == 0);
        assert(alu.srcB.is_reg()? alu.srcB.reg().tag == ACC && alu.srcB.reg().regId == 5 : true);
        uint32_t raddrb;

        if (alu.srcB.is_reg()) {
          raddrb = 48;
        } else {
          uint32_t n = (uint32_t) alu.srcB.imm().val;
          assert(n >= 1 || n <= 15);
          raddrb = 48 + n;
        }


        vc4_instr.tag(vc4_Instr::ROT);
        vc4_instr.mulOp  = ALUOp(ALUOp::M_V8MIN).vc4_encodeMulOp();
        vc4_instr.raddrb = raddrb;
      } else {
        uint32_t muxa, muxb;
        uint32_t raddra = 0, raddrb;

        if (alu.srcA.is_reg() && alu.srcB.is_reg()) { // Both operands are registers
          RegTag aFile = regFileOf(alu.srcA.reg());
          RegTag aTag  = alu.srcA.reg().tag;

          RegTag bFile = regFileOf(alu.srcB.reg());
          RegTag bTag  = alu.srcB.reg().tag;

          // If operands are the same register
          if (aTag != NONE && aTag == bTag && alu.srcA.reg().regId == alu.srcB.reg().regId) {
            if (aFile == REG_A) {
              raddra = encodeSrcReg(alu.srcA.reg(), REG_A, &muxa);
              muxb = muxa; raddrb = 39;
            } else {
              raddrb = encodeSrcReg(alu.srcA.reg(), REG_B, &muxa);
              muxb = muxa; raddra = 39;
            }
          } else {
            // Operands are different registers
            assert(aFile == NONE || bFile == NONE || aFile != bFile);  // TODO examine why aFile == bFile is disallowed here
            if (aFile == REG_A || bFile == REG_B) {
              raddra = encodeSrcReg(alu.srcA.reg(), REG_A, &muxa);
              raddrb = encodeSrcReg(alu.srcB.reg(), REG_B, &muxb);
            } else {
              raddrb = encodeSrcReg(alu.srcA.reg(), REG_B, &muxa);
              raddra = encodeSrcReg(alu.srcB.reg(), REG_A, &muxb);
            }
          }
        } else if (alu.srcA.is_imm() || alu.srcB.is_imm()) {
          if (alu.srcA.is_imm() && alu.srcB.is_imm()) {
            //assertq(false , "srcA and srcB can not both be immediates", true);

          /* Not working */

            assertq(alu.srcA.imm().val == alu.srcB.imm().val,
                    "srcA and srcB can not both be immediates with different values", true);

            raddrb = (uint32_t) alu.srcA.imm().val;  // srcB is the same
            muxa   = 7;
            muxb   = 7;
          } else if (alu.srcB.is_imm()) {
            // Second operand is a small immediate
            raddra = encodeSrcReg(alu.srcA.reg(), REG_A, &muxa);
            raddrb = (uint32_t) alu.srcB.imm().val;
            muxb   = 7;
          } else if (alu.srcA.is_imm()) {
            // First operand is a small immediate
            raddra = encodeSrcReg(alu.srcB.reg(), REG_A, &muxb);
            raddrb = (uint32_t) alu.srcA.imm().val;
            muxa   = 7;
          } else {
            assert(false);  // Not expecting this
          }
        } else {
          assert(false);  // Not expecting this
        }

      
        vc4_instr.tag(vc4_Instr::ALU, instr.hasImm());
        vc4_instr.mulOp = (alu.op.isMul() ? alu.op.vc4_encodeMulOp() : 0);
        vc4_instr.addOp = (alu.op.isMul() ? 0 : alu.op.vc4_encodeAddOp());
        vc4_instr.raddra  = raddra;
        vc4_instr.raddrb  = raddrb;
        vc4_instr.muxa  = muxa;
        vc4_instr.muxb  = muxb;
      }
    }
    break;

    // Halt
    case END:
      vc4_instr.tag(vc4_Instr::END);
      break;

    case TMU0_TO_ACC4:
      vc4_instr.tag(vc4_Instr::TMU0_TO_ACC4);
      break;

    // Semaphore increment/decrement
    case SINC:
      vc4_instr.tag(vc4_Instr::SINC);
      vc4_instr.sema_id = instr.semaId;
      break;

    case SDEC:
      vc4_instr.tag(vc4_Instr::SDEC);
      vc4_instr.sema_id = instr.semaId;
      break;

    // No-op & ignored instructions
    case NO_OP:
    case INIT_BEGIN:
    case INIT_END:
      break; // Use default value for instr, which is a full NOP

    default:
      fatal("V3DLib: missing case in vc4 encodeInstr");
      break;
  }

  *high = vc4_instr.high();
  *low  = vc4_instr.low();
}

}  // anon namespace


// ============================================================================
// Top-level encoder
// ============================================================================

uint64_t encode(Instr instr) {
  uint32_t low;
  uint32_t high;

  encodeInstr(instr, &high, &low);

  uint64_t ret = (((uint64_t) high) << 32) + low;
  return ret;
}


void encode(Instr::List &instrs, UIntList &code) {
  uint32_t high, low;

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs.get(i);
    check_instruction_tag_for_platform(instr.tag, true);

    if (instr.tag == INIT_BEGIN || instr.tag == INIT_END) {
      continue;  // Don't encode these block markers
    }

    encodeInstr(instr, &high, &low);
    code << low << high;
  }
}

}  // namespace vc4
}  // namespace V3DLib
