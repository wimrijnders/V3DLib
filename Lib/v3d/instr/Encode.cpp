#include "Encode.h"
#include "Support/Exception.h"
#include "Mnemonics.h"

namespace V3DLib {
namespace v3d {
namespace instr {
namespace {

uint8_t const NOP_ADDR    = 39;
uint8_t const REGB_OFFSET = 32;
uint8_t const NUM_REGS_RF = 64;  // Number of available registers in the register file


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


void check_reg(Reg reg) {
  if (reg.regId < 0) {
    error("Unassigned regId value", true);
  }

  if (reg.regId >= NUM_REGS_RF) {
    breakpoint
    error("regId value out of range", true);
  }
}


}  // anon namespace

uint8_t to_waddr(Reg const &reg) {
  assertq(reg.tag != REG_B, "to_waddr(): Not expecting REG_B any more, examine");
  assert(reg.tag == REG_A);
  return (uint8_t) (reg.regId);
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

}  // instr
}  // v3d
}  // V3DLib

