#ifndef _V3DLIB_SOURCE_OP_H_
#define _V3DLIB_SOURCE_OP_H_
#include <string>
#include "Target/instr/ALUOp.h"

namespace V3DLib {

class OpItem;

// ============================================================================
// Operators
// ============================================================================

// Every operator has a type associated with it.
// Also used in other operator/comparison classes.
enum BaseType { UINT8, INT16, INT32, FLOAT };

enum OpId {
  // Int & Float operators:
  ROTATE, ADD, SUB, MUL, MIN, MAX,

  // Int only operators:
  SHL, SHR, USHR, BOR, BAND, BXOR, BNOT, ROR,

  // Conversion operators:
  ItoF, FtoI,

  // SFU functions
  RECIP,
  RECIPSQRT,
  EXP,
  LOG,

  // Other combined operators
  SIN,

  // v3d only
  TIDX,
  EIDX,
  FFLOOR,
};


/**
 * Pair containing operator and base type
 *
 * This data class has degenerated to a facade for OpItem.
 * TODO cleanup and/or consolidate
 */
struct Op {
  OpId op;
  BaseType type;

  Op(Op const &rhs);
  Op(OpId in_op, BaseType in_type);

  bool isUnary() const;

  std::string disp(std::string const &lhs, std::string const &rhs) const;
  std::string dump() const;

  ALUOp::Enum opcode() const;

private:
  OpItem const &m_item;
};

}  // namespace V3DLib

#endif  //  _V3DLIB_SOURCE_OP_H_
