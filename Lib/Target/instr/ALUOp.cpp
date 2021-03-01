#include "ALUOp.h"
#include <stdint.h>
#include "Support/Platform.h"
#include "Source/Op.h"
#include "Support/basics.h"

namespace V3DLib {

ALUOp::ALUOp(Op const &op) : m_value(opcode(op)) {}


/**
 * Translate source operator to target opcode
 */
ALUOp::Enum ALUOp::opcode(Op const &op) const {
  if (op.type == FLOAT) {
    switch (op.op) {
      case ADD:    return A_FADD;
      case SUB:    return A_FSUB;
      case MUL:    return M_FMUL;
      case MIN:    return A_FMIN;
      case MAX:    return A_FMAX;
      case ItoF:   return A_ItoF;
      case ROTATE: return M_ROTATE;
      //case FFLOOR: return M_FFLOOR;
      default:     assert(false);
    }
  } else {
    switch (op.op) {
      case ADD:    return A_ADD;
      case SUB:    return A_SUB;
      case MUL:    return M_MUL24;
      case MIN:    return A_MIN;
      case MAX:    return A_MAX;
      case FtoI:   return A_FtoI;
      case SHL:    return A_SHL;
      case SHR:    return A_ASR;
      case USHR:   return A_SHR;
      case ROR:    return A_ROR;
      case BAND:   return A_BAND;
      case BOR:    return A_BOR;
      case BXOR:   return A_BXOR;
      case BNOT:   return A_BNOT;
      case ROTATE: return M_ROTATE;
      case TIDX: 
        assertq(!Platform::compiling_for_vc4(), "opcode(): TIDX is only for v3d", true);
        return A_TIDX;
      case EIDX: 
        assertq(!Platform::compiling_for_vc4(), "opcode(): EIDX is only for v3d", true);
        return A_EIDX;
      default:
        assertq(false, "Not expecting this op for int in opcode()", true);
        break;
    }
  }

  return NOP;
}


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


std::string ALUOp::pretty() const {
  std::string ret;

  ret << pretty_op();
  return ret;
}


char const *ALUOp::pretty_op() const {
  switch (m_value) {
    case NOP:       return "nop";
    case A_FADD:    return "addf";
    case A_FSUB:    return "subf";
    case A_FMIN:    return "minf";
    case A_FMAX:    return "maxf";
    case A_FMINABS: return "minabsf";
    case A_FMAXABS: return "maxabsf";
    case A_FtoI:    return "ftoi";
    case A_ItoF:    return "itof";
    case A_ADD:     return "add";
    case A_SUB:     return "sub";
    case A_SHR:     return "shr";
    case A_ASR:     return "asr";
    case A_ROR:     return "ror";
    case A_SHL:     return "shl";
    case A_MIN:     return "min";
    case A_MAX:     return "max";
    case A_BAND:    return "and";
    case A_BOR:     return "or";
    case A_BXOR:    return "xor";
    case A_BNOT:    return "not";
    case A_CLZ:     return "clz";
    case A_V8ADDS:  return "addsatb";
    case A_V8SUBS:  return "subsatb";
    case M_FMUL:    return "mulf";
    case M_MUL24:   return "mul24";
    case M_V8MUL:   return "mulb";
    case M_V8MIN:   return "minb";
    case M_V8MAX:   return "maxb";
    case M_V8ADDS:  return "m_addsatb";
    case M_V8SUBS:  return "m_subsatb";
    case M_ROTATE:  return "rotate";
    // v3d-specific
    case A_TIDX:    return "tidx";
    case A_EIDX:    return "eidx";
    default:
      assertq(false, "pretty_op(): Unknown alu opcode", true);
      return "";
  }
}


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
      fatal("V3DLib: unknown add op");
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
