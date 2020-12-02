//
// This module defines the abstract syntax of the QPU language.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_SOURCE_SYNTAX_H_
#define _V3DLIB_SOURCE_SYNTAX_H_
#include "Int.h"

namespace V3DLib {

// Direction for VPM/DMA loads and stores
enum Dir { HORIZ, VERT };

// Reserved general-purpose vars
enum ReservedVarId : VarId {
  RSV_QPU_ID   = 0,
  RSV_NUM_QPUS = 1
};


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
	BExpr() {}
	//BExpr(BExpr const &rhs); 
	BExpr(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs);

	BExprTag tag() const { return m_tag; }
  Expr::Ptr cmp_lhs() const;
  Expr::Ptr cmp_rhs() const;
  void cmp_lhs(Expr::Ptr p);
  void cmp_rhs(Expr::Ptr p);

	BExpr *Not() const;
	BExpr *And(BExpr *rhs) const;
	BExpr *Or(BExpr *rhs) const;

	std::string pretty() const;
	std::string disp() const { return pretty(); }

  union {
    // Negation
    BExpr* neg;

    // Conjunction
    struct { BExpr* lhs; BExpr* rhs; } conj;

    // Disjunction
    struct { BExpr* lhs; BExpr* rhs; } disj;

    // Comparison
    struct {
			CmpOp op;
		} cmp;
  };

private:
  BExprTag m_tag;
  Expr::Ptr m_cmp_lhs;  // For comparison
	Expr::Ptr m_cmp_rhs;  // idem
};


// ============================================================================
// Conditional expressions
// ============================================================================

// Kinds of conditional expressions
enum CExprTag { ALL, ANY };

struct CExpr {
	CExpr(CExprTag tag, BExpr *bexpr) : m_tag(tag), m_bexpr(bexpr)  {}

  BExpr *bexpr() const { return m_bexpr; }
  CExprTag tag() const { return m_tag; }

private:

  // What kind of boolean expression is it?
  CExprTag m_tag;

  // This is either a scalar boolean expression, or a reduction of a vector
  // boolean expressions using 'any' or 'all' operators.
  BExpr *m_bexpr = nullptr;
};

// Functions to construct conditional expressions
CExpr* mkAll(BExpr* bexpr);
CExpr* mkAny(BExpr* bexpr);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_SYNTAX_H_
