#ifndef _V3DLIB_TARGET_INSTR_INSTRUCTION_H_
#define _V3DLIB_TARGET_INSTR_INSTRUCTION_H_
#include "Conditions.h"
#include "RegOrImm.h"
#include "ALUOp.h"

namespace V3DLib {

struct ALUInstruction {
  SetCond    m_setCond;
  RegOrImm   srcA;
  ALUOp      op;
  RegOrImm   srcB;

  ALUInstruction() = default;

  bool noOperands() const;
  bool oneOperand() const;
};

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_INSTR_INSTRUCTION_H_
