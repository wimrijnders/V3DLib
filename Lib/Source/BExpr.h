#ifndef _V3DLIB_SOURCE_BEXPR_H_
#define _V3DLIB_SOURCE_BEXPR_H_
#include <memory>
#include <string>
#include "Expr.h"
#include "Target/instr/Conditions.h"

namespace V3DLib {

// ============================================================================
// Class CmpOp
// ============================================================================

/**
 * Comparison operators
 *
 * Pair containing comparison operator and base type.
 */
class CmpOp {
public:
  enum Id { EQ, NEQ, LT, GT, LE, GE };

  CmpOp() : m_op(EQ), m_type(INT32) {}  // Arbitrary initializers
  CmpOp(Id op, BaseType type) : m_op(op), m_type(type) {}

  Id op() const { return m_op; }
  void op(Id val) { m_op = val; }
  BaseType type() const { return m_type; }

  SetCond::Tag cond_tag() const;
  Flag         assign_flag() const;
  char const  *to_string() const;

private:
  Id       m_op;
  BaseType m_type;
};


// ============================================================================
// Class BExpr
// ============================================================================

// Kinds of boolean expressions
enum BExprTag { NOT, AND, OR, CMP };

/**
 * Boolean expressions
 */
struct BExpr {
  using Ptr = std::shared_ptr<BExpr>;

  BExpr() {}
  BExpr(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs);

  BExprTag tag() const { return m_tag; }

  Ptr neg() const;
  Ptr lhs() const;
  Ptr rhs() const;
  Expr::Ptr cmp_lhs() const;
  Expr::Ptr cmp_rhs() const;
  void cmp_lhs(Expr::Ptr p);
  void cmp_rhs(Expr::Ptr p);

  Ptr Not() const;
  Ptr And(Ptr rhs) const;
  Ptr Or(Ptr rhs) const;

  std::string dump() const;

  CmpOp cmp;

private:
  BExprTag m_tag;
  Ptr m_lhs;
  Ptr m_rhs;
  Expr::Ptr m_cmp_lhs;  // For comparison
  Expr::Ptr m_cmp_rhs;  // idem

  Ptr ptr() const;
};

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_BEXPR_H_
