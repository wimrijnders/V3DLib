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

  Var var();

  std::string pretty() const;
  std::string dump() const;

  union {
    int   intLit;   // Integer literal
    float floatLit; // Float literal
    Var   m_var;      // Variable identifier
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
  BaseExpr();

  Expr::Ptr expr() const { return m_expr; }
  std::string dump() const;

protected:
  Expr::Ptr m_expr;  // Abstract syntax tree

  BaseExpr(char const *label);
  BaseExpr(Expr::Ptr e, char const *label = "");

  void assign_intern();
  void assign_intern(Expr::Ptr expr);
  Expr::Ptr deref_with_index(Expr::Ptr base, Expr::Ptr index_expr);

private:
  char const *m_label = "";
};


// ============================================================================
// class IntExpr                   
// ============================================================================

/**
 * An 'IntExpr' defines an integer vector expression which can
 * only be used on the RHS of assignment statements.
 */
class IntExpr : public BaseExpr {
public:
  IntExpr(int x);
  IntExpr(Expr::Ptr e) : BaseExpr(e) {}
};


// Functions to construct expressions
Expr::Ptr mkIntLit(int lit);
Expr::Ptr mkVar(Var var);
Expr::Ptr mkApply(Expr::Ptr lhs, Op op, Expr::Ptr rhs);
Expr::Ptr mkApply(Expr::Ptr rhs, Op op);
Expr::Ptr mkDeref(Expr::Ptr ptr);

}  // namespace V3DLib


#endif  //  _V3DLIB_SOURCE_EXPR_H_
