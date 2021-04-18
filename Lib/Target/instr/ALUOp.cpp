#include "ALUOp.h"
#include <stdint.h>
#include "Support/basics.h"
#include "Source/Op.h"

namespace V3DLib {
namespace {

char const *pretty_op(ALUOp::Enum value) {
  using Enum = ALUOp::Enum;

  switch (value) {
    case Enum::NOP:       return "nop";
    case Enum::A_FADD:    return "addf";
    case Enum::A_FSUB:    return "subf";
    case Enum::A_FMIN:    return "minf";
    case Enum::A_FMAX:    return "maxf";
    case Enum::A_FMINABS: return "minabsf";
    case Enum::A_FMAXABS: return "maxabsf";
    case Enum::A_FtoI:    return "ftoi";
    case Enum::A_ItoF:    return "itof";
    case Enum::A_ADD:     return "add";
    case Enum::A_SUB:     return "sub";
    case Enum::A_SHR:     return "shr";
    case Enum::A_ASR:     return "asr";
    case Enum::A_ROR:     return "ror";
    case Enum::A_SHL:     return "shl";
    case Enum::A_MIN:     return "min";
    case Enum::A_MAX:     return "max";
    case Enum::A_BAND:    return "and";
    case Enum::A_BOR:     return "or";
    case Enum::A_BXOR:    return "xor";
    case Enum::A_BNOT:    return "not";
    case Enum::A_CLZ:     return "clz";
    case Enum::A_V8ADDS:  return "addsatb";
    case Enum::A_V8SUBS:  return "subsatb";
    case Enum::M_FMUL:    return "fmul";
    case Enum::M_MUL24:   return "mul24";
    case Enum::M_V8MUL:   return "mulb";
    case Enum::M_V8MIN:   return "minb";
    case Enum::M_V8MAX:   return "maxb";
    case Enum::M_V8ADDS:  return "m_addsatb";
    case Enum::M_V8SUBS:  return "m_subsatb";
    case Enum::M_ROTATE:  return "rotate";
    // v3d-specific
    case Enum::A_TIDX:    return "tidx";
    case Enum::A_EIDX:    return "eidx";
    case Enum::A_FFLOOR:  return "ffloor";
    case Enum::A_FSIN:    return "sin";
    default:
      assertq(false, "pretty_op(): Unknown alu opcode", true);
      return "";
  }
}

}  // anon namespace


ALUOp::ALUOp(Op const &op) : m_value(op.opcode()) {}


/**
 * Determines if the mul-ALU needs to be used
 *
 * TODO: Examine if this is still true for v3d
 */
bool ALUOp::isMul() const {
  auto op = m_value;

  bool ret =
    op == M_FMUL   || op == M_MUL24 || op == M_V8MUL  ||
    op == M_V8MIN  || op == M_V8MAX || op == M_V8ADDS ||
    op == M_V8SUBS || op == M_ROTATE;

  return ret;
}


std::string ALUOp::pretty() const { return pretty_op(m_value); }


uint32_t ALUOp::vc4_encodeAddOp() const {
  switch (m_value) {
    case NOP:       return 0;
    case A_FADD:    return 1;
    case A_FSUB:    return 2;
    case A_FMIN:    return 3;
    case A_FMAX:    return 4;
    case A_FMINABS: return 5;
    case A_FMAXABS: return 6;
    case A_FtoI:    return 7;
    case A_ItoF:    return 8;
    case A_ADD:     return 12;
    case A_SUB:     return 13;
    case A_SHR:     return 14;
    case A_ASR:     return 15;
    case A_ROR:     return 16;
    case A_SHL:     return 17;
    case A_MIN:     return 18;
    case A_MAX:     return 19;
    case A_BAND:    return 20;
    case A_BOR:     return 21;
    case A_BXOR:    return 22;
    case A_BNOT:    return 23;
    case A_CLZ:     return 24;
    case A_V8ADDS:  return 30;
    case A_V8SUBS:  return 31;

    default:
      assertq("V3DLib: unknown add op", true);
      return 0;
  }
}


uint32_t ALUOp::vc4_encodeMulOp() const {
  switch (m_value) {
    case NOP:      return 0;
    case M_FMUL:   return 1;
    case M_MUL24:  return 2;
    case M_V8MUL:  return 3;
    case M_V8MIN:  return 4;
    case M_V8MAX:  return 5;
    case M_V8ADDS: return 6;
    case M_V8SUBS: return 7;

    default:
      fatal("V3DLib: unknown mul op");
      return 0;
  }
}



}  // namespace V3DLib
