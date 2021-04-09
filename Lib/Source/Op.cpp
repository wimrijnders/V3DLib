#include "Op.h"
#include <vector>
#include "Support/debug.h"
#include "OpItems.h"

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

    case SIN:    return "sin";

    // v3d-specific
    case TIDX:   return "tidx";
    case EIDX:   return "eidx";
    case FFLOOR:  return "ffloor";

    default:
      OpItem const *item = OpItems::find(op);
      if (item != nullptr) {
        return item->str;
      }
      assertq(false, "Unknown OpId in Op::to_string()");
  }

  assertq(false, "opToString(): unknown opcode", true);
  return nullptr;
}


bool Op::noParams() const {
  return (op == TIDX  || op == EIDX);
}


bool Op::isUnary() const {
  OpItem const *item = OpItems::find(op);
  if (item != nullptr) {
    return item->is_unary();
  }

  return (op == BNOT || op == ItoF || op == FtoI || op == FFLOOR || isFunction());
}


bool Op::isFunction() const {
  OpItem const *item = OpItems::find(op);
  if (item != nullptr) {
    return item->is_function;
  }

  return (op == RECIP || op == RECIPSQRT || op == EXP || op == LOG || op == FFLOOR);
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
