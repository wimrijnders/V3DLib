#ifndef _V3DLIB_SOURCE_EXPR_H_
#define _V3DLIB_SOURCE_EXPR_H_
#include <memory>
#include "Var.h"
#include "Op.h"

namespace V3DLib {

// ============================================================================
// Expressions    
// ============================================================================


struct Expr {
	using Ptr = std::shared_ptr<Expr>;

	enum Tag {
		INT_LIT,
		FLOAT_LIT,
		VAR,
		APPLY,
		DEREF       // Dereference a pointer
	};

	Expr();
	Expr(Var in_var);
	Expr(int in_lit);
	Expr(float in_lit);
	Expr(Ptr in_lhs, Op op, Ptr in_rhs);
	Expr(Ptr ptr);

	Tag tag() const { return m_tag; }
	bool isLit() const { return (tag() == INT_LIT) || (tag() == FLOAT_LIT); }

  Ptr lhs() const;
  Ptr rhs() const;
  Ptr deref_ptr() const;
  void lhs(Ptr p);
  void rhs(Ptr p);
  void deref_ptr(Ptr p);

	std::string pretty() const;
	std::string disp() const;

  union {
    int   intLit;   // Integer literal
    float floatLit; // Float literal
    Var   var;      // Variable identifier
    Op apply_op;    // Application of a binary operator
  };

	bool isSimple() const;

private:
  Tag m_tag;    // What kind of expression is it?
  Ptr m_exp_a;  // lhs for apply, ptr for deref
	Ptr m_exp_b;  // rhs for apply

	std::string disp_apply() const;
};



class BaseExpr {
public:
	BaseExpr() {}
	BaseExpr(Expr::Ptr e, char const *label = "");

	Expr::Ptr expr() const { return m_expr; }
	void set_with_index(Expr::Ptr base, Expr::Ptr index_expr);

	std::string disp() const;

protected:
  Expr::Ptr m_expr;  // Abstract syntax tree

private:
	char const *m_label = "";
};


// Functions to construct expressions
Expr::Ptr mkIntLit(int lit);
Expr::Ptr mkVar(Var var);
Expr::Ptr mkApply(Expr::Ptr lhs, Op op, Expr::Ptr rhs);
Expr::Ptr mkDeref(Expr::Ptr ptr);

}  // namespace V3DLib


#endif  //  _V3DLIB_SOURCE_EXPR_H_
