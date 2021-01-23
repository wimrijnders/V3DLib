#include "Expr.h"
#include "Target/SmallLiteral.h"
#include "Support/basics.h"
#include "Source/Lang.h"  // assign()

namespace V3DLib {

using ::operator<<;  // C++ weirdness

Expr::Expr() {
	breakpoint
}


Expr::Expr(Var in_var) {
	m_tag = VAR; 
	m_var   = in_var;
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


Var Expr::var() {
  assertq(m_tag == VAR, "Expr is not a VAR, shouldn't access var member.", true);
	return m_var;
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
	} else if (apply_op.isFunction()) {
		ret << apply_op.to_string() << "(" << lhs()->pretty() << ")";
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
    case VAR:       ret << m_var.disp();                 break;
    case APPLY:     ret << disp_apply();                 break;
    case DEREF:     ret << "*" << deref_ptr()->pretty(); break;
		default:
			assert(false);
		break;
	}

	return ret;
}


std::string Expr::dump() const {
	std::string ret;

	switch(m_tag) {
		case INT_LIT:   ret << "Int "    << intLit;              break;
		case FLOAT_LIT: ret << "Float "  << floatLit;            break;
		case VAR:       ret << "Var: "   << m_var.disp();        break;
		case APPLY:     ret << "Apply: " << disp_apply();        break;
		case DEREF:     ret << "Deref: " << deref_ptr()->dump(); break;
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
 * Create deref with added index offset to base
 *
 * Meant for Ptr-types
 */
Expr::Ptr BaseExpr::deref_with_index(Expr::Ptr base, Expr::Ptr index_expr) {
	return mkDeref(mkApply(base, Op(ADD, INT32), mkApply(index_expr, Op(SHL, INT32), mkIntLit(2))));
}


std::string BaseExpr::dump() const {
	std::string ret;
	ret << m_label << " " << m_expr->dump(); 
	return ret;
}


// ============================================================================
// Functions on expressions
// ============================================================================

Expr::Ptr mkIntLit(int lit) { return std::make_shared<Expr>(lit); }
Expr::Ptr mkVar(Var var) { return std::make_shared<Expr>(var); }
Expr::Ptr mkDeref(Expr::Ptr ptr) { return std::make_shared<Expr>(ptr); }


/**
 * Binary op version
 *
 * Unary operation can be passed in, the second parameter 'rhs'
 * will be ignored in the assembly.
 */
Expr::Ptr mkApply(Expr::Ptr lhs, Op op, Expr::Ptr rhs) {
	return std::make_shared<Expr>(lhs, op, rhs);
}


/**
 * Unary op version
 */
Expr::Ptr mkApply(Expr::Ptr lhs, Op op) {
	assert(op.isUnary());
	return std::make_shared<Expr>(lhs, op, mkIntLit(0));
}


}  // namespace V3DLib
