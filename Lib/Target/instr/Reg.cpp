#include "Reg.h"
#include "Support/basics.h"
#include "Mnemonics.h"

namespace V3DLib {
namespace {

const char* specialStr(RegId rid) {
  Special s = (Special) rid;
  switch (s) {
    case SPECIAL_UNIFORM:       return "UNIFORM";
    case SPECIAL_ELEM_NUM:      return "ELEM_NUM";
    case SPECIAL_QPU_NUM:       return "QPU_NUM";
    case SPECIAL_RD_SETUP:      return "RD_SETUP";
    case SPECIAL_WR_SETUP:      return "WR_SETUP";
    case SPECIAL_DMA_ST_ADDR:   return "DMA_ST_ADDR";
    case SPECIAL_DMA_LD_ADDR:   return "DMA_LD_ADDR";
    case SPECIAL_DMA_ST_WAIT:   return "DMA_ST_WAIT";
    case SPECIAL_DMA_LD_WAIT:   return "DMA_LD_WAIT";
    case SPECIAL_VPM_READ:      return "VPM_READ";
    case SPECIAL_VPM_WRITE:     return "VPM_WRITE";
    case SPECIAL_HOST_INT:      return "HOST_INT";
    case SPECIAL_TMU0_S:        return "TMU0_S";
    case SPECIAL_SFU_RECIP:     return "SFU_RECIP";
    case SPECIAL_SFU_RECIPSQRT: return "SFU_RECIPSQRT";
    case SPECIAL_SFU_EXP:       return "SFU_EXP";
    case SPECIAL_SFU_LOG:       return "SFU_LOG";
  }

  // Unreachable
  assert(false);
  return "";
}
}  //  anon namespace


/**
 * Translate variable to source register.
 */
Reg srcReg(Var v) {
  Reg r;

  switch (v.tag()) {
    case UNIFORM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_UNIFORM;
      r.isUniformPtr = v.is_uniform_ptr();
      break;
    case QPU_NUM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_QPU_NUM;
      break;
    case ELEM_NUM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_ELEM_NUM;
      break;
    case VPM_READ:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_VPM_READ;
      break;
    case STANDARD:
      r.tag   = REG_A;
      r.regId = v.id();
      break;
    case VPM_WRITE:
    case TMU0_ADDR:
      printf("V3DLib: Reading from write-only special register is forbidden\n");
      assert(false);
      break;
    case DUMMY:
      r.tag   = NONE;
      r.regId = v.id();
      break;
    default:
      assert(false);
      break;
  }

  return r;
}


/**
 * Translate variable to target register.
 */
Reg dstReg(Var v) {
  using namespace V3DLib::Target::instr;

  switch (v.tag()) {
    case UNIFORM:
    case QPU_NUM:
    case ELEM_NUM:
    case VarTag::VPM_READ:
      fatal("V3DLib: writing to read-only special register is forbidden");
      return Reg();  // Return anything

    case STANDARD:          return Reg(REG_A, v.id());
    case VarTag::VPM_WRITE: return Target::instr::VPM_WRITE;
    case TMU0_ADDR:         return TMU0_S;

    default:
      fatal("Unhandled case in dstReg()");
      return Reg();  // Return anything
  }
}


std::string Reg::dump() const {
  std::string ret;

  switch (tag) {
    case REG_A:   ret <<   "A" << regId; break;
    case REG_B:   ret <<   "B" << regId; break;
    case ACC:     ret << "ACC" << regId; break;
    case SPECIAL: ret <<  "S[" << specialStr(regId) << "]"; break;
    case NONE:    ret <<   "_"; break;

    default: assertq(false, "Reg::dump() failed", true); break;
  }

  return ret;
}


// TODO Move this away, to DMA
bool is_dma_only_register(Reg const &reg) {
  if (reg.tag != SPECIAL) return false;

  switch (reg.regId) {
    case SPECIAL_VPM_READ:
    case SPECIAL_DMA_ST_WAIT:
    case SPECIAL_DMA_LD_WAIT:
    case SPECIAL_RD_SETUP:
    case SPECIAL_WR_SETUP:
    case SPECIAL_HOST_INT:
    case SPECIAL_DMA_LD_ADDR:
      return true;
  }

  return false;
}

}  // namespace V3DLib
