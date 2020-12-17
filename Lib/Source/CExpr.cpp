#include "CExpr.h"

namespace V3DLib {


std::string CExpr::pretty() const {
	std::string ret;

  switch (m_tag) {
    // Reduce using 'any'
    case ANY: ret << "any("; break;

    // Reduce using 'all'
    case ALL: ret << "all("; break;
  }

	ret << bexpr()->pretty() << ")";

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
