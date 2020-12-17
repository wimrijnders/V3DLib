#ifndef _V3DLIB_SOURCE_BEXPR_H_
#define _V3DLIB_SOURCE_BEXPR_H_
#include <memory>
#include "Expr.h"
//#include "Op.h"  // BaseType

namespace V3DLib {


// ============================================================================
// Comparison operators
// ============================================================================

// Comparison operators
enum CmpOpId { EQ, NEQ, LT, GT, LE, GE };

// Pair containing comparison operator and base type
struct CmpOp {
	CmpOpId op;
	BaseType type;

	CmpOp(CmpOpId in_op, BaseType in_type) : op(in_op), type(in_type) {}

	char const *to_string() const;
};

// ============================================================================
// Boolean expressions
// ============================================================================


// Kinds of boolean expressions
enum BExprTag { NOT, AND, OR, CMP };

struct BExpr {
	using Ptr = std::shared_ptr<BExpr>;

	BExpr() {}
	BExpr(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs);

	BExprTag tag() const { return m_tag; }

  Ptr neg() const;
  Ptr conj_lhs() const;
  Ptr conj_rhs() const;
  Ptr disj_lhs() const;
  Ptr disj_rhs() const;
  Ptr lhs() const;
  Ptr rhs() const;
  Expr::Ptr cmp_lhs() const;
  Expr::Ptr cmp_rhs() const;
  void cmp_lhs(Expr::Ptr p);
  void cmp_rhs(Expr::Ptr p);

	Ptr Not() const;
	Ptr And(Ptr rhs) const;
	Ptr Or(Ptr rhs) const;

	std::string pretty() const;
	std::string disp() const { return pretty(); }

  union {
    // Negation
    //BExpr* neg;

    // Conjunction
    //struct { BExpr* lhs; BExpr* rhs; } conj;

    // Disjunction
    //struct { BExpr* lhs; BExpr* rhs; } disj;

    // Comparison
    struct {
			CmpOp op;
		} cmp;
  };

private:
  BExprTag m_tag;
  Ptr m_lhs;
	Ptr m_rhs;
  Expr::Ptr m_cmp_lhs;  // For comparison
	Expr::Ptr m_cmp_rhs;  // idem
};

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_BEXPR_H_
