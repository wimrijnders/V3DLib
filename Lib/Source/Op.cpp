#include "Op.h"
#include "Support/Platform.h"
#include "Support/debug.h"

namespace V3DLib {

const char *Op::to_string() const {
	switch (op) {
		case ADD:    return "+";
		case SUB:    return "-";
		case MUL:    return "*";
		case MIN:    return " min ";
		case MAX:    return " max ";
		case ROTATE: return " rotate ";
		case SHL:    return " << ";
		case SHR:    return " >> ";
		case USHR:   return " _>> ";
		case ROR:    return " ror ";
		case BOR:    return " | ";
		case BAND:   return " & ";
		case BXOR:   return " ^ ";
		case BNOT:   return "~";
		case ItoF:   return "(Float) ";
		case FtoI:   return "(Int) ";

		// SFU functions
		case RECIP:  return "recip";
		case RECIPSQRT: return "recipsqrt";
		case EXP:    return "exp";
		case LOG:    return "log";

		// v3d-specific
		case TIDX:   return "tidx";
		case EIDX:   return "eidx";
	}

	assertq(false, "opToString(): unknown opcode", true);
	return nullptr;
}


bool Op::noParams() const {
  return (op == TIDX  || op == EIDX);
}


bool Op::isUnary() const {
  return (op == BNOT || op == ItoF || op == FtoI || isFunction());
}


bool Op::isFunction() const {
  return (op == RECIP || op == RECIPSQRT || op == EXP || op == LOG);
}


// Is given operator commutative?
bool Op::isCommutative() const {
  if (type != FLOAT) {
    return op == ADD
        || op == MUL
        || op == BOR
        || op == BAND
        || op == BXOR
        || op == MIN
        || op == MAX;
  }

  return false;
}


/**
 * Translate source operator to target opcode
 */
ALUOp Op::opcode() const {
  if (type == FLOAT) {
    switch (op) {
      case ADD:    return A_FADD;
      case SUB:    return A_FSUB;
      case MUL:    return M_FMUL;
      case MIN:    return A_FMIN;
      case MAX:    return A_FMAX;
      case ItoF:   return A_ItoF;
      case ROTATE: return M_ROTATE;
      default:     assert(false);
    }
  }
  else {
    switch (op) {
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
				assertq(!Platform::instance().compiling_for_vc4(), "opcode(): TIDX is only for v3d", true);
				return A_TIDX;
      case EIDX: 
				assertq(!Platform::instance().compiling_for_vc4(), "opcode(): EIDX is only for v3d", true);
				return A_EIDX;
      default:
				assertq(false, "Not expecting this op for int in opcode()", true);
				break;
    }
  }

	return NOP;
}

}  // namespace V3DLib
