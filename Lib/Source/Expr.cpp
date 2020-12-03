#include "Expr.h"
#include "Target/SmallLiteral.h"
#include "Support/basics.h"

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


Expr::Expr(Ptr in_lhs, Op op, Ptr in_rhs) {
  m_tag = APPLY;
  lhs(in_lhs);
  apply_op = op;
  rhs(in_rhs);
}


Expr::Expr(Ptr ptr) {
  m_tag = DEREF;
  deref_ptr(ptr);
}


Expr::Ptr Expr::lhs() const {
	assert(m_tag == APPLY && m_exp_a.get() != nullptr);
	return m_exp_a;
}


Expr::Ptr Expr::rhs() const {
	assert(m_tag == APPLY && m_exp_b.get() != nullptr);
	return m_exp_b;
}


Expr::Ptr Expr::deref_ptr() const {
	assert(m_tag == DEREF && m_exp_a.get() != nullptr);
	return m_exp_a;
}


void Expr::lhs(Ptr p) {
	assert(m_tag == APPLY);
	m_exp_a = p;
}


void Expr::rhs(Ptr p) {
	assert(m_tag == APPLY);
	m_exp_b = p;
}


void Expr::deref_ptr(Ptr p) {
	assert(m_tag == DEREF);
	m_exp_a = p;
}


std::string Expr::disp_apply() const {
	assert(tag() == APPLY);
	std::string ret;

	if (apply_op.noParams()) {
		ret << apply_op.to_string() << "()";
	} else if (apply_op.isUnary()) {
		ret << "(" << apply_op.to_string() << lhs()->pretty() << ")";
	} else {
		ret << "(" << lhs()->pretty() << apply_op.to_string() << rhs()->pretty() <<  ")";
	}

	return ret;
}


std::string Expr::pretty() const {
	std::string ret;

  switch (tag()) {
    case INT_LIT:   ret << intLit;                       break;
    case FLOAT_LIT: ret << floatLit;                     break;
    case VAR:       ret << var.disp();                   break;
    case APPLY:     ret << disp_apply();                 break;
    case DEREF:     ret << "*" << deref_ptr()->pretty(); break;
		default:
			assert(false);
		break;
	}

	return ret;
}


std::string Expr::disp() const {
	std::string ret;

	switch(m_tag) {
		case INT_LIT:   ret << "Int "    << intLit;              break;
		case FLOAT_LIT: ret << "Float "  << floatLit;            break;
		case VAR:       ret << "Var: "   << var.disp();          break;
		case APPLY:     ret << "Apply: " << disp_apply();        break;
		case DEREF:     ret << "Deref: " << deref_ptr()->disp(); break;
		default:
			assert(false);
		break;
	}

	return ret;
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

BaseExpr::BaseExpr(Expr::Ptr e, char const *label) : m_label(label) {
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


std::string BaseExpr::disp() const {
	std::string ret;
	ret << m_label << " " << m_expr->disp(); 
	return ret;
}


// ============================================================================
// Functions on expressions
// ============================================================================

Expr::Ptr mkIntLit(int lit) { return std::make_shared<Expr>(lit); }
Expr::Ptr mkVar(Var var) { return std::make_shared<Expr>(var); }
Expr::Ptr mkApply(Expr::Ptr lhs, Op op, Expr::Ptr rhs) { return std::make_shared<Expr>(lhs, op, rhs); }
Expr::Ptr mkDeref(Expr::Ptr ptr) { return std::make_shared<Expr>(ptr); }

}  // namespace V3DLib
