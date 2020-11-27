#ifndef _V3DLIB_SOURCE_EXPR_H_
#define _V3DLIB_SOURCE_EXPR_H_
#include <memory>
#include "Var.h"
#include "Op.h"

namespace V3DLib {

// ============================================================================
// Expressions    
// ============================================================================

enum ExprTag {
	INT_LIT,
	FLOAT_LIT,
	VAR,
	APPLY,
	DEREF       // Dereference a pointer
};


struct Expr {
	using ExprPtr = std::shared_ptr<Expr>;

	Expr();
	Expr(Var in_var);
	Expr(int in_lit);
	Expr(float in_lit);
	Expr(ExprPtr lhs, Op op, ExprPtr rhs);
	Expr(ExprPtr ptr);

	~Expr();

	ExprTag tag() const { return m_tag; }
	bool isLit() const { return (tag() == INT_LIT) || (tag() == FLOAT_LIT); }

  ExprPtr apply_lhs();
  ExprPtr apply_rhs();
  ExprPtr &deref_ptr();
  void apply_lhs(ExprPtr p);
  void apply_rhs(ExprPtr p);
  void deref_ptr(ExprPtr p);

  union {
    int   intLit;                                   // Integer literal
    float floatLit;                                 // Float literal
    Var   var;                                      // Variable identifier

    struct { Op op; } apply;  // Application of a binary operator
  };

	bool isSimple() const;

private:
  ExprTag m_tag;  // What kind of expression is it?

  ExprPtr m_exp_a;  // lhs for apply, ptr for deref
	ExprPtr m_exp_b;  // rhs for apply
};


using ExprPtr = std::shared_ptr<Expr>;


class BaseExpr {
public:
	BaseExpr() {}
	BaseExpr(ExprPtr e);
	~BaseExpr();

	ExprPtr expr() const { return m_expr; }

protected:
  ExprPtr m_expr;  // Abstract syntax tree
};


// Functions to construct expressions
ExprPtr mkIntLit(int lit);
ExprPtr mkVar(Var var);
ExprPtr mkApply(ExprPtr lhs, Op op, ExprPtr rhs);
ExprPtr mkDeref(ExprPtr ptr);

}  // namespace V3DLib


#endif  //  _V3DLIB_SOURCE_EXPR_H_
