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

Expr::Expr(ExprPtr lhs, Op op, ExprPtr rhs) {
  m_tag     = APPLY;
  apply_lhs(lhs);
  apply.op  = op;
  apply_rhs(rhs);
}


Expr::Expr(ExprPtr ptr) {
  m_tag     = DEREF;
  deref_ptr(ptr);
}


Expr::~Expr() {
	breakpoint
}


ExprPtr Expr::apply_lhs() {
	assert(m_tag == APPLY && m_exp_a.get() != nullptr);
	return m_exp_a;
}


ExprPtr Expr::apply_rhs() {
	assert(m_tag == APPLY && m_exp_b.get() != nullptr);
	return m_exp_b;
}


ExprPtr &Expr::deref_ptr() {
	breakpoint  // TODO returning ref prob wrong, check where used
	assert(m_tag == DEREF && m_exp_a.get() != nullptr);
	return m_exp_a;
}


void Expr::apply_lhs(ExprPtr p) {
	assert(m_tag == APPLY);
	m_exp_a = p;
}


void Expr::apply_rhs(ExprPtr p) {
	assert(m_tag == APPLY);
	m_exp_b = p;
}

void Expr::deref_ptr(ExprPtr p) {
	assert(m_tag == DEREF);
	m_exp_a = p;
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

BaseExpr::BaseExpr(ExprPtr e) {
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
ExprPtr mkIntLit(int lit) {
  return new Expr(lit);
}


// Make a variable
ExprPtr mkVar(Var var) {
  return new Expr(var);
}

// Make an operator application
ExprPtr mkApply(ExprPtr lhs, Op op, ExprPtr rhs) {
  return new Expr(lhs, op, rhs);
}


// Make a pointer dereference
ExprPtr mkDeref(ExprPtr ptr) {
  return new Expr(ptr);
}


}  // namespace V3DLib
