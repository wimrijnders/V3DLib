#include "CExpr.h"
#include "Support/basics.h"

namespace V3DLib {

std::string CExpr::pretty() const {
	assert(m_tag == ALL || m_tag == ANY);
	assert(m_bexpr.get() != nullptr);

	std::string ret;
	ret << ((m_tag == ALL)?"all":"any") << "(" << m_bexpr->pretty() << ")";
	return ret;
}


// ============================================================================
// Functions on conditionals
// ============================================================================

CExpr::Ptr mkAll(BExpr::Ptr bexpr) {
  return CExpr::Ptr(new CExpr(ALL, bexpr));
}


CExpr::Ptr mkAny(BExpr::Ptr bexpr) {
  return CExpr::Ptr(new CExpr(ANY, bexpr));
}

}  // namespace V3DLib
