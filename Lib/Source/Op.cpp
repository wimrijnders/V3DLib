#include "Op.h"
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

}  // namespace V3DLib
