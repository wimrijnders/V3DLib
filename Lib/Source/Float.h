// This module defines type 'Float' for a vector of 16 x 32-bit floats.

#ifndef _V3DLIB_SOURCE_FLOAT_H_
#define _V3DLIB_SOURCE_FLOAT_H_

#include <assert.h>
#include "Source/Syntax.h"

namespace V3DLib {

// ============================================================================
// Types                   
// ============================================================================

// An 'FloatExpr' defines an float vector expression which can
// only be used on the RHS of assignment statements.

struct FloatExpr :public BaseExpr {
  FloatExpr(float x);
	FloatExpr(Expr *e) : BaseExpr(e) {}
};


// An 'Float' defines a float vector variable which can be used in
// both the LHS and RHS of an assignment.

struct Float : public BaseExpr {
  Float();
  Float(float x);
  Float(FloatExpr e);

  // Copy constructors
  Float(Float& x);
  Float(const Float& x);

  // Cast to an FloatExpr
  operator FloatExpr();

  // Assignment
  Float& operator=(Float& rhs);
  FloatExpr operator=(FloatExpr rhs);
};


// Helper constructor

// TODO get rid of this
inline FloatExpr mkFloatExpr(Expr* e) { return FloatExpr(e); }

// ============================================================================
// Operations
// ============================================================================

FloatExpr getUniformFloat();
FloatExpr vpmGetFloat();

FloatExpr operator+(FloatExpr a, FloatExpr b);
FloatExpr operator-(FloatExpr a, FloatExpr b);
FloatExpr operator*(FloatExpr a, FloatExpr b);
FloatExpr min(FloatExpr a, FloatExpr b);
FloatExpr max(FloatExpr a, FloatExpr b);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_FLOAT_H_
