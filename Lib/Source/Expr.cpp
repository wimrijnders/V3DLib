#include "Expr.h"
#include "Target/SmallLiteral.h"
#include "Support/debug.h"

namespace V3DLib {

Expr::Expr() {
	breakpoint
}


Expr::Expr(Var in_var) {
	m_tag = VAR; 
	var   = in_var;
}


Expr::Expr(int in_lit) {
	m_tag = INT_LIT; 
  intLit = in_lit;
}


Expr::Expr(float in_lit) {
	m_tag = FLOAT_LIT; 
  floatLit = in_lit;
}

Expr::Expr(Expr* lhs, Op op, Expr* rhs) {
  m_tag     = APPLY;
  apply.lhs = lhs;
  apply.op  = op;
  apply.rhs = rhs;
}


Expr::Expr(Expr* ptr) {
  m_tag     = DEREF;
  deref.ptr = ptr;
}


Expr::~Expr() {
	breakpoint
}


/**
 * An expression is 'simple' if it is a small literal or a variable.
 */
bool Expr::isSimple() const {
	bool isSmallLit = encodeSmallLit(*this) >= 0;
  return (m_tag == VAR) || isSmallLit;
}


// ============================================================================
// Class BaseExpr
// ============================================================================

BaseExpr::BaseExpr(Expr *e) {
	assert(e != nullptr);
	m_expr = e;
}


BaseExpr::~BaseExpr() {
	breakpoint
}

// ============================================================================
// Functions on expressions
// ============================================================================

// Make an integer literal
Expr* mkIntLit(int lit) {
  return new Expr(lit);
}

// Make a float literal
Expr* mkFloatLit(float lit) {
  return new Expr(lit);
}

// Make a variable
Expr* mkVar(Var var) {
  return new Expr(var);
}

// Make an operator application
Expr* mkApply(Expr* lhs, Op op, Expr* rhs) {
  return new Expr(lhs, op, rhs);
}


// Make a pointer dereference
Expr* mkDeref(Expr* ptr) {
  return new Expr(ptr);
}


}  // namespace V3DLib
