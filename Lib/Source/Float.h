// This module defines type 'Float' for a vector of 16 x 32-bit floats.

#ifndef _V3DLIB_SOURCE_FLOAT_H_
#define _V3DLIB_SOURCE_FLOAT_H_
#include "Source/Expr.h"

namespace V3DLib {

struct Float;
template <typename T> struct Deref; // Forward declaration template class

// ============================================================================
// Types                   
// ============================================================================

// An 'FloatExpr' defines an float vector expression which can
// only be used on the RHS of assignment statements.

struct FloatExpr :public BaseExpr {
  FloatExpr(float x);
	FloatExpr(Expr::Ptr e) : BaseExpr(e) {}
	FloatExpr(Deref<Float> d);
};


// An 'Float' defines a float vector variable which can be used in
// both the LHS and RHS of an assignment.

struct Float : public BaseExpr {
  Float();
  Float(float x);
  Float(FloatExpr e);
  Float(Deref<Float> d);

  // Copy constructors
  Float(Float& x);
  Float(Float const &x);

  // Cast to an FloatExpr
  operator FloatExpr();

  // Assignment
  Float& operator=(Float& rhs);
  FloatExpr operator=(FloatExpr rhs);
};


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

// SFU functions
FloatExpr recip(FloatExpr x);
FloatExpr recipsqrt(FloatExpr x);
FloatExpr exp(FloatExpr x);
FloatExpr log(FloatExpr x);  

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_FLOAT_H_
