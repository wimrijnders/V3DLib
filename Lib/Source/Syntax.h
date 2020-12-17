//
// This module defines the abstract syntax of the QPU language.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_SOURCE_SYNTAX_H_
#define _V3DLIB_SOURCE_SYNTAX_H_
#include "Int.h"
#include "BExpr.h"

namespace V3DLib {

// Direction for VPM/DMA loads and stores
enum Dir { HORIZ, VERT };

// Reserved general-purpose vars
enum ReservedVarId : VarId {
  RSV_QPU_ID   = 0,
  RSV_NUM_QPUS = 1
};


// ============================================================================
// Conditional expressions
// ============================================================================

// Kinds of conditional expressions
enum CExprTag { ALL, ANY };

struct CExpr {
	CExpr(CExprTag tag, BExpr::Ptr bexpr) : m_tag(tag), m_bexpr(bexpr)  {}

  BExpr::Ptr bexpr() const { return m_bexpr; }
  CExprTag tag() const { return m_tag; }

private:

  // What kind of boolean expression is it?
  CExprTag m_tag;

  // This is either a scalar boolean expression, or a reduction of a vector
  // boolean expressions using 'any' or 'all' operators.
  BExpr::Ptr m_bexpr;
};

// Functions to construct conditional expressions
CExpr* mkAll(BExpr::Ptr bexpr);
CExpr* mkAny(BExpr::Ptr bexpr);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_SYNTAX_H_
