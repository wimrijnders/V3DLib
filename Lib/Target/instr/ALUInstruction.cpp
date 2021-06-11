#include "ALUInstruction.h"
#include "Support/debug.h"

namespace V3DLib {

bool ALUInstruction::noOperands() const {
  if (!srcA.is_reg() || !srcB.is_reg()) return false;
  if (srcA.reg().tag != NONE || srcB.reg().tag != NONE) return false;

  // Pedantry: these should be the only operations with no operands
  assert(op.value() == ALUOp::A_TIDX || op.value() == ALUOp::A_EIDX);
  return true;
}


bool ALUInstruction::oneOperand() const {
  if (!srcA.is_reg() || srcA.reg().tag == NONE) return false;
  if (!(srcB.is_reg() && srcB.reg().tag == NONE)) return false;

  // Pedantry: these should be the only operations with one operand
  assert(op.value() == ALUOp::A_FSIN || op.value() == ALUOp::A_FFLOOR);
  return true;
}


}  // namespace V3DLib
