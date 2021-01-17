#include "BExpr.h"
#include "Support/basics.h"

namespace V3DLib {

// ============================================================================
// Class CmpOp
// ============================================================================

SetCond::Tag CmpOp::cond_tag() const {
  SetCond::Tag ret = SetCond::NO_COND;

  switch (m_op) {
  	case EQ:  ret = SetCond::Z; break;
  	case NEQ: ret = SetCond::Z; break;
  	case LT:  ret = SetCond::N; break;
  	case GE:  ret = SetCond::N; break;
  	default:  assert(false);
  }

  return ret;
}


Flag CmpOp::assign_flag() const {
  Flag ret = ZS;  // Arbitrary initializer

  switch(m_op) {
  	case EQ:  ret = ZS; break;
  	case NEQ: ret = ZC; break;
  	case LT:  ret = NS; break;
  	case GE:  ret = NC; break;
  	default:  assert(false);
  }

  return ret;
}


char const *CmpOp::to_string() const {
  switch (m_op) {
    case EQ : return "==";
    case NEQ: return "!=";
    case LT : return "<";
    case LE : return "<=";
    case GT : return ">";
    case GE : return ">=";
  }

  // Not reachable
  assert(false);
  return nullptr;
}


// ============================================================================
// Class BExpr
// ============================================================================

BExpr::BExpr(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs) {
 	m_tag      = CMP;
  m_cmp_lhs  = lhs;
  cmp.op     = op;
  m_cmp_rhs  = rhs;
}


BExpr::Ptr BExpr::neg() const {
  assert(m_tag == NOT && m_lhs.get() != nullptr);
  assert(m_rhs.get() == nullptr);  // Should not be used with negate
  return m_lhs;
}


BExpr::Ptr BExpr::lhs() const {
  assert(m_tag == OR || m_tag == AND);
  assert(m_lhs.get() != nullptr);
  return m_lhs;
}


BExpr::Ptr BExpr::rhs() const {
  assert(m_tag == OR || m_tag == AND);
  assert(m_rhs.get() != nullptr);
  return m_rhs;
}


Expr::Ptr BExpr::cmp_lhs() const { assert(m_tag == CMP && m_cmp_lhs.get() != nullptr); return  m_cmp_lhs; }
Expr::Ptr BExpr::cmp_rhs() const { assert(m_tag == CMP && m_cmp_rhs.get() != nullptr); return  m_cmp_rhs; }

void BExpr::cmp_lhs(Expr::Ptr p) { assert(m_tag == CMP); m_cmp_lhs = p; }
void BExpr::cmp_rhs(Expr::Ptr p) { assert(m_tag == CMP); m_cmp_rhs = p; }

/**
 * Return copy of current instance as a shared ptr.
 */
BExpr::Ptr BExpr::ptr() const {
  return Ptr(new BExpr(*this));  // Verified correct (looked tricky)
}

/**
 * Create a new instance which encapsulates the current instance with
 * a negation.
 *
 * `not` is a keyword, hence capital.
 */
BExpr::Ptr BExpr::Not() const {
  Ptr b(new BExpr());
  b->m_tag = NOT;
  b->m_lhs = ptr();
  return b;
}


/**
 * Create a new instance which combines the current instance with
 * the passed instance to an and-operation.
 *
 * `and` is a keyword, hence capital.
 */
BExpr::Ptr BExpr::And(Ptr rhs) const {
  Ptr b(new BExpr());
  b->m_tag = AND;
  b->m_lhs = ptr();
  b->m_rhs = rhs;
  return b;
}


/**
 * Create a new instance which combines the current instance with
 * the passed instance to an or-operation.
 *
 * `or` is a keyword, hence capital.
 */
BExpr::Ptr BExpr::Or(Ptr rhs) const {
  Ptr b(new BExpr());
  b->m_tag = OR;
  b->m_lhs = ptr();
  b->m_rhs = rhs;
  return b;
}


std::string BExpr::pretty() const {
  using ::operator<<;  // C++ weirdness

  std::string ret;

  switch (tag()) {
    // Negation
    case NOT:
  		assert(m_lhs.get() != nullptr);
  		ret << "!" << m_lhs->pretty();
      break;

    // Conjunction
    case AND:
  		assert(m_lhs.get() != nullptr);
  		assert(m_rhs.get() != nullptr);
      ret << "(" << m_lhs->pretty() << " && " << m_rhs->pretty() << ")";
      break;

    // Disjunction
    case OR:
  		assert(m_lhs.get() != nullptr);
  		assert(m_rhs.get() != nullptr);
  		ret << "(" << m_lhs->pretty() << " || " << m_rhs->pretty() << ")";
      break;

    // Comparison
    case CMP:
  		ret << cmp_lhs()->pretty() << cmp.op.to_string() << cmp_rhs()->pretty();
      break;
  }

  return ret;
}

}  // namespace V3DLib
