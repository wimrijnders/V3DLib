#ifndef _V3DLIB_SOURCE_CEXPR_H_
#define _V3DLIB_SOURCE_CEXPR_H_
#include <memory>
#include <string>
#include "BExpr.h"

namespace V3DLib {

// ============================================================================
// Conditional expressions
// ============================================================================

// Kinds of conditional expressions
enum CExprTag { ALL, ANY };

struct CExpr {
	using Ptr = std::shared_ptr<CExpr>;

	CExpr(CExprTag tag, BExpr::Ptr bexpr) : m_tag(tag), m_bexpr(bexpr)  {}

  BExpr::Ptr bexpr() const { return m_bexpr; }
  CExprTag tag() const { return m_tag; }

	std::string pretty() const;

private:

  // What kind of boolean expression is it?
  CExprTag m_tag;

  // This is either a scalar boolean expression, or a reduction of a vector
  // boolean expressions using 'any' or 'all' operators.
  BExpr::Ptr m_bexpr;
};

// Functions to construct conditional expressions
CExpr::Ptr mkAll(BExpr::Ptr bexpr);
CExpr::Ptr mkAny(BExpr::Ptr bexpr);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_CEXPR_H_
