#include "Syntax.h"
#include "Common/Stack.h"
#include "Support/basics.h"

namespace V3DLib {

/*
BExpr::	BExpr(BExpr const &rhs) {
	switch(rhs.tag() {
	case NOT:
		break;
	case AND:
		break;
	case OR:
		break;
	case CMP:
		break;
	default:
		assert(false);
		break;
	}
}
*/


BExpr::BExpr(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs) {
 	m_tag      = CMP;
  m_cmp_lhs  = lhs;
  cmp.op   = op;
  m_cmp_rhs  = rhs;
}


Expr::Ptr BExpr::cmp_lhs() { assert(m_tag == CMP && m_cmp_lhs.get() != nullptr); return  m_cmp_lhs; }
Expr::Ptr BExpr::cmp_rhs() { assert(m_tag == CMP && m_cmp_rhs.get() != nullptr); return  m_cmp_rhs; }

void BExpr::cmp_lhs(Expr::Ptr p) { assert(m_tag == CMP); m_cmp_lhs = p; }
void BExpr::cmp_rhs(Expr::Ptr p) { assert(m_tag == CMP); m_cmp_rhs = p; }


/**
 * Create a new instance which encapsulates the current instance with
 * a negation.
 *
 * `not` is a keyword, hence capital.
 */
BExpr *BExpr::Not() const {
  BExpr *b = new BExpr();
  b->m_tag = NOT;
  b->neg   = const_cast<BExpr *>(this);
  return b;
}


/**
 * Create a new instance which combines the current instance with
 * the passed instance to an and-operation.
 *
 * `and` is a keyword, hence capital.
 */
BExpr *BExpr::And(BExpr *rhs) const {
  BExpr *b = new BExpr();
  b->m_tag = AND;
  b->conj.lhs = const_cast<BExpr *>(this);
  b->conj.rhs = rhs;
  return b;
}


/**
 * Create a new instance which combines the current instance with
 * the passed instance to an or-operation.
 *
 * `or` is a keyword, hence capital.
 */
BExpr *BExpr::Or(BExpr *rhs) const {
  BExpr *b    = new BExpr();
  b->m_tag     = OR;
  b->disj.lhs = const_cast<BExpr *>(this);
  b->disj.rhs = rhs;
  return b;
}


// ============================================================================
// Functions on conditionals
// ============================================================================

CExpr *mkCExpr() {
	breakpoint
	return new CExpr;
}

CExpr* mkAll(BExpr* bexpr) {
  CExpr* c = mkCExpr();
  c->tag   = ALL;
  c->bexpr = bexpr;
  return c;
}


CExpr* mkAny(BExpr* bexpr) {
  CExpr* c = mkCExpr();
  c->tag   = ANY;
  c->bexpr = bexpr;
  return c;
}
}  // namespace V3DLib
