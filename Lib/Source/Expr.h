#ifndef _V3DLIB_SOURCE_EXPR_H_
#define _V3DLIB_SOURCE_EXPR_H_
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
	DEREF
};


struct Expr {
	Expr();
	Expr(Var in_var);
	Expr(int in_lit);
	Expr(float in_lit);
	Expr(Expr* lhs, Op op, Expr* rhs);
	Expr(Expr* ptr);

	~Expr();

	ExprTag tag() const { return m_tag; }
	bool isLit() const { return (tag() == INT_LIT) || (tag() == FLOAT_LIT); }

  union {
    int   intLit;                                   // Integer literal
    float floatLit;                                 // Float literal
    Var   var;                                      // Variable identifier

    struct { Expr* lhs; Op op; Expr* rhs; } apply;  // Application of a binary operator
    struct { Expr* ptr; } deref;                    // Dereference a pointer
  };

	bool isSimple() const;

private:
  ExprTag m_tag;  // What kind of expression is it?
};


class BaseExpr {
public:
	BaseExpr() {}
	BaseExpr(Expr *e);
	~BaseExpr();

	Expr *expr() const { return m_expr; }

protected:
  Expr *m_expr = nullptr;  // Abstract syntax tree
};


// Functions to construct expressions
Expr* mkIntLit(int lit);
Expr* mkFloatLit(float lit);
Expr* mkVar(Var var);
Expr* mkApply(Expr* lhs, Op op, Expr* rhs);
Expr* mkDeref(Expr* ptr);

}  // namespace V3DLib


#endif  //  _V3DLIB_SOURCE_EXPR_H_
