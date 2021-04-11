#ifndef _V3DLIB_TARGET_SYNTAX_INSTR_ALUOP_H_
#define _V3DLIB_TARGET_SYNTAX_INSTR_ALUOP_H_
#include <string>
#include "Source/Op.h"

namespace V3DLib {

class ALUOp {
public:
  //
  // Prefixes:
  // A_: for 'add' ALU
  // M_: for 'mul' ALU
  //
  enum Enum :uint32_t {
    // The idea behind this construct is to eventually be able to use
    // one enum for OpIDs and ALUOp::Enum
    FLOAT = 0x80000000, // signals float operation
    NONE = (uint32_t) -1,

    NOP = 0,            // No op

    // Opcodes for the 'add' ALU
    A_FADD,         // Floating-point add
    A_FSUB,         // Floating-point subtract
    A_FMIN,         // Floating-point min
    A_FMAX,         // Floating-point max
    A_FMINABS,      // Floating-point min of absolute values
    A_FMAXABS,      // Floating-point max of absolute values
    A_FtoI,         // Float to signed integer
    A_ItoF,         // Signed integer to float
    A_ADD,          // Integer add
    A_SUB,          // Integer subtract
    A_SHR,          // Integer shift right
    A_ASR,          // Integer arithmetic shift right
    A_ROR,          // Integer rotate right
    A_SHL,          // Integer shift left
    A_MIN,          // Integer min
    A_MAX,          // Integer max
    A_BAND,         // Bitwise and
    A_BOR,          // Bitwise or
    A_BXOR,         // Bitwise xor
    A_BNOT,         // Bitwise not
    A_CLZ,          // Count leading zeros
    A_V8ADDS,       // Add with saturation per 8-bit element
    A_V8SUBS,       // Subtract with saturation per 8-bit element

    // Opcodes for the 'mul' ALU
    M_FMUL,        // Floating-point multiply
    M_MUL24,       // 24-bit integer multiply
    M_V8MUL,       // Multiply per 8-bit element
    M_V8MIN,       // Min per 8-bit element
    M_V8MAX,       // Max per 8-bit element
    M_V8ADDS,      // Add with saturation per 8-bit element
    M_V8SUBS,      // Subtract with saturation per 8-bit element
    M_ROTATE,      // Rotation (intermediate op-code)

    // v3d only
    A_TIDX,
    A_EIDX,
    A_FFLOOR,
    A_FSIN
/*
    // Example of use 'FLOAT'
    NUM_OPS,

    // NB don't implement this! using recip() instead
    A_FDIV = NUM_OPS + FLOAT + OpId::DIV, // Floating-point divide
*/
  };

  explicit ALUOp(Enum val) : m_value(val) {}
  explicit ALUOp(Op const &op);

  Enum value() const { return m_value; }
  bool isNOP() const { return m_value == NOP; }
  bool isRot() const { return m_value == M_ROTATE; }
  bool isMul() const;
  bool noOperands() const { return (m_value == A_TIDX || m_value == A_EIDX); }
  std::string pretty() const;

  uint32_t vc4_encodeAddOp() const;
  uint32_t vc4_encodeMulOp() const;

  bool operator==(ALUOp::Enum rhs) const { return m_value == rhs; }

private:
  Enum m_value = NOP;

  char const *pretty_op() const;
};

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_SYNTAX_INSTR_ALUOP_H_
