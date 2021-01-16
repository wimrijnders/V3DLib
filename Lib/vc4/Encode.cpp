#include "Encode.h"
#include <stdio.h>
#include <stdlib.h>
#include "Support/basics.h"  // fatal()
#include "Target/Satisfy.h"
#include "Target/Pretty.h"


namespace V3DLib {
namespace vc4 {

namespace {

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
  // Convert intermediate instruction into core instruction
  switch (instr.tag) {
    case IRQ:
      instr.tag           = LI;
      instr.LI.m_setCond.clear();
      instr.LI.cond.tag   = AssignCond::Tag::ALWAYS;
      instr.LI.dest.tag   = SPECIAL;
      instr.LI.dest.regId = SPECIAL_HOST_INT;
      instr.LI.imm.tag    = IMM_INT32;
      instr.LI.imm.intVal = 1;
      break;

    case DMA_LOAD_WAIT:
    case DMA_STORE_WAIT: {
      RegId src = instr.tag == DMA_LOAD_WAIT ? SPECIAL_DMA_LD_WAIT : SPECIAL_DMA_ST_WAIT;

      instr.tag                   = ALU;
      instr.ALU.m_setCond.clear();
      instr.ALU.cond.tag          = AssignCond::Tag::NEVER;
      instr.ALU.op                = ALUOp(ALUOp::A_BOR);
      instr.ALU.dest.tag          = NONE;
      instr.ALU.srcA.tag          = REG;
      instr.ALU.srcA.reg.tag      = SPECIAL;
      instr.ALU.srcA.reg.regId    = src;
      instr.ALU.srcB.tag          = REG;
      instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
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
      *high         = 0xe0000000 | cond | ws | sf | waddr_add | waddr_mul;
      *low          = (uint32_t) li.imm.intVal;
      return;
    }

    // Branch
    case BR: {
      // Register offset not yet supported
      assert(! instr.BR.target.useRegOffset);

      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t cond = encodeBranchCond(instr.BR.cond) << 20;
      uint32_t rel  = (instr.BR.target.relative ? 1 : 0) << 19;
      *high = 0xf0000000 | cond | rel | waddr_add | waddr_mul;
      *low  = (uint32_t) 8*instr.BR.target.immOffset;
      return;
    }

    // ALU
    case ALU: {
			auto &alu = instr.ALU;

      RegTag file;
      uint32_t sig   = ((instr.hasImm() || alu.op.isRot()) ? 13 : 1) << 28;
      uint32_t cond  = encodeAssignCond(alu.cond) << (alu.op.isMul() ? 14 : 17);
      uint32_t dest  = encodeDestReg(alu.dest, &file);
      uint32_t waddr_add, waddr_mul, ws;

      if (alu.op.isMul()) {
        waddr_add = 39 << 6;
        waddr_mul = dest;
        ws        = (file == REG_B ? 0 : 1) << 12;
      } else {
        waddr_add = dest << 6;
        waddr_mul = 39;
        ws        = (file == REG_A ? 0 : 1) << 12;
      }

      uint32_t sf    = (alu.m_setCond.flags_set()? 1 : 0) << 13;
      *high          = sig | cond | ws | sf | waddr_add | waddr_mul;

      if (alu.op.isRot()) {
        assert(alu.srcA.tag == REG && alu.srcA.reg.tag == ACC && alu.srcA.reg.regId == 0);
        assert(alu.srcB.tag == REG ?  alu.srcB.reg.tag == ACC && alu.srcB.reg.regId == 5 : true);
        uint32_t mulOp = ALUOp(ALUOp::M_V8MIN).vc4_encodeMulOp() << 29;
        uint32_t raddrb;

        if (alu.srcB.tag == REG) {
          raddrb = 48;
        } else {
          uint32_t n = (uint32_t) alu.srcB.smallImm.val;
          assert(n >= 1 || n <= 15);
          raddrb = 48 + n;
        }

        uint32_t raddra = 39;
        *low = mulOp | (raddrb << 12) | (raddra << 18);
        return;
      } else {
        uint32_t mulOp = (alu.op.isMul() ? alu.op.vc4_encodeMulOp() : 0) << 29;
        uint32_t addOp = (alu.op.isMul() ? 0 : alu.op.vc4_encodeAddOp()) << 24;

        uint32_t muxa, muxb;
        uint32_t raddra, raddrb;

        // Both operands are registers
        if (alu.srcA.tag == REG && alu.srcB.tag == REG) {
          RegTag aFile = regFileOf(alu.srcA.reg);
          RegTag bFile = regFileOf(alu.srcB.reg);
          RegTag aTag  = alu.srcA.reg.tag;
          RegTag bTag  = alu.srcB.reg.tag;

          // If operands are the same register
          if (aTag != NONE && aTag == bTag && alu.srcA.reg.regId == alu.srcB.reg.regId) {
            if (aFile == REG_A) {
              raddra = encodeSrcReg(alu.srcA.reg, REG_A, &muxa);
              muxb = muxa; raddrb = 39;
            } else {
              raddrb = encodeSrcReg(alu.srcA.reg, REG_B, &muxa);
              muxb = muxa; raddra = 39;
            }
          } else {
            // Operands are different registers
            assert(aFile == NONE || bFile == NONE || aFile != bFile);  // TODO examine why aFile == bFile is disallowed here
            if (aFile == REG_A || bFile == REG_B) {
              raddra = encodeSrcReg(alu.srcA.reg, REG_A, &muxa);
              raddrb = encodeSrcReg(alu.srcB.reg, REG_B, &muxb);
            } else {
              raddrb = encodeSrcReg(alu.srcA.reg, REG_B, &muxa);
              raddra = encodeSrcReg(alu.srcB.reg, REG_A, &muxb);
            }
          }
        } else if (alu.srcB.tag == IMM) {
          // Second operand is a small immediate
          raddra = encodeSrcReg(alu.srcA.reg, REG_A, &muxa);
          raddrb = (uint32_t) alu.srcB.smallImm.val;
          muxb   = 7;
        } else if (alu.srcA.tag == IMM) {
          // First operand is a small immediate
          raddra = encodeSrcReg(alu.srcB.reg, REG_A, &muxb);
          raddrb = (uint32_t) alu.srcA.smallImm.val;
          muxa   = 7;
        } else {
          // Both operands are small immediates
          assert(false);
        }

        *low = mulOp | addOp | (raddra << 18) | (raddrb << 12)
                     | (muxa << 9) | (muxb << 6)
                     | (muxa << 3) | muxb;
        return;
      }
    }

    // Halt
    case END:
    case TMU0_TO_ACC4: {
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t raddra = 39 << 18;
      uint32_t raddrb = 39 << 12;
      uint32_t sig = instr.tag == END ? 0x30000000 : 0xa0000000;
      *high  = sig | waddr_add | waddr_mul;
      *low   = raddra | raddrb;
      return;
    }

    // Semaphore increment/decrement
    case SINC:
    case SDEC: {
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t sig = 0xe8000000;
      uint32_t incOrDec = (instr.tag == SINC ? 0 : 1) << 4;
      *high = sig | waddr_add | waddr_mul;
      *low = incOrDec | instr.semaId;
      return;
    }

    // No-op & ignored instructions
    case NO_OP:
    case PRI:
    case PRS:
    case PRF:
		case INIT_BEGIN:
		case INIT_END: {
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      *high  = 0xe0000000 | waddr_add | waddr_mul;
      *low   = 0;
      return;
		}

		default:
  		fatal("V3DLib: missing case in vc4 encodeInstr");
			return;
  }
}

}  // anon namespace

// =================
// Top-level encoder
// =================

uint64_t encode(Instr instr) {
	uint64_t ret;
	uint32_t low;
	uint32_t high;

	encodeInstr(instr, &high, &low);

	ret = (((uint64_t) high) << 32) + low;

	return ret;
}

void encode(Seq<Instr>* instrs, Seq<uint32_t>* code) {
  uint32_t high, low;
  for (int i = 0; i < instrs->size(); i++) {
    Instr instr = instrs->get(i);
		check_instruction_tag_for_platform(instr.tag, true);

		if (instr.tag == INIT_BEGIN || instr.tag == INIT_END) {
			continue;  // Don't encode these block markers
		}

    encodeInstr(instr, &high, &low);
    *code << low << high;
  }
}

}  // namespace vc4
}  // namespace V3DLib
