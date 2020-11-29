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


Expr::Expr(Ptr lhs, Op op, Ptr rhs) {
  m_tag     = APPLY;
  apply_lhs(lhs);
  apply.op  = op;
  apply_rhs(rhs);
}


Expr::Expr(Ptr ptr) {
  m_tag     = DEREF;
  deref_ptr(ptr);
}


Expr::Ptr Expr::apply_lhs() {
	assert(m_tag == APPLY && m_exp_a.get() != nullptr);
	return m_exp_a;
}


Expr::Ptr Expr::apply_rhs() {
	assert(m_tag == APPLY && m_exp_b.get() != nullptr);
	return m_exp_b;
}


Expr::Ptr Expr::deref_ptr() {
	assert(m_tag == DEREF && m_exp_a.get() != nullptr);
	return m_exp_a;
}


void Expr::apply_lhs(Ptr p) {
	assert(m_tag == APPLY);
	m_exp_a = p;
}


void Expr::apply_rhs(Ptr p) {
	assert(m_tag == APPLY);
	m_exp_b = p;
}


void Expr::deref_ptr(Ptr p) {
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

BaseExpr::BaseExpr(Expr::Ptr e) {
	assert(e != nullptr);
	m_expr = e;
}


/**
 * Set instance expression to base with added index offset
 *
 * Meant for Ptr-types
 */
void BaseExpr::set_with_index(Expr::Ptr base, Expr::Ptr index_expr) {
	Expr::Ptr e = mkDeref(mkApply(base, Op(ADD, INT32), mkApply(index_expr, Op(SHL, INT32), mkIntLit(2))));
	m_expr = e;
}


// ============================================================================
// Functions on expressions
// ============================================================================

Expr::Ptr mkIntLit(int lit) { return std::make_shared<Expr>(lit); }
Expr::Ptr mkVar(Var var) { return std::make_shared<Expr>(var); }
Expr::Ptr mkApply(Expr::Ptr lhs, Op op, Expr::Ptr rhs) { return std::make_shared<Expr>(lhs, op, rhs); }
Expr::Ptr mkDeref(Expr::Ptr ptr) { return std::make_shared<Expr>(ptr); }

}  // namespace V3DLib
