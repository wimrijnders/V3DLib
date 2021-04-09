///////////////////////////////////////////////////////////////////////////////
// This module defines type 'Float' for a vector of 16 x 32-bit floats.
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_SOURCE_FLOAT_H_
#define _V3DLIB_SOURCE_FLOAT_H_
#include "Common/Seq.h"
#include "Expr.h"
#include "Ptr.h"
#include "Int.h"

namespace V3DLib {

struct Float;
template <typename T> struct Deref; // Forward declaration template class

// ============================================================================
// Types                   
// ============================================================================

/**
 * A 'FloatExpr' defines an float vector expression which can
 * only be used on the RHS of assignment statements.
 */
struct FloatExpr :public BaseExpr {
  FloatExpr(float x);
  FloatExpr(Expr::Ptr e) : BaseExpr(e) {}
  FloatExpr(Deref<Float> d);

  IntExpr as_int() { return IntExpr(m_expr); }  //<< Reinterpret the float expression as an int expression
  FloatExpr operator-();
};


// An 'Float' defines a float vector variable which can be used in
// both the LHS and RHS of an assignment.

struct Float : public BaseExpr {
  using Array   = V3DLib::SharedArray<float>;
  using Array2D = V3DLib::Shared2DArray<float>;
  using Ptr     = V3DLib::ptr::Ptr<Float>;

  Float();
  Float(float x);
  Float(FloatExpr e);
  Float(Deref<Float> d);
  Float(Float const &x);

  static Float mkArg();
  static bool passParam(IntList &uniforms, float val);

  void as_float(IntExpr rhs);
  operator FloatExpr() const;

  // Assignment
  Float &operator=(float rhs);
  Float &operator=(Float &rhs);
  Float &operator=(Float const &rhs);
  FloatExpr operator=(FloatExpr const &rhs);
  Float &operator=(Deref<Float> d);
  Float &operator+=(FloatExpr rhs);
  Float &operator-=(FloatExpr rhs);
  Float &operator*=(FloatExpr rhs);

  void set_at(Int n, Float const &src);

private:
  Float &self();  // NB: 'me()' as name didn't work here, global me() got used instead in .cpp
};


// ============================================================================
// Operations
// ============================================================================

FloatExpr vpmGetFloat();

FloatExpr rotate(FloatExpr a, IntExpr b);
IntExpr toInt(FloatExpr a);
FloatExpr toFloat(IntExpr a);
FloatExpr ffloor(FloatExpr a);

FloatExpr operator+(FloatExpr a, FloatExpr b);
FloatExpr operator-(FloatExpr a, FloatExpr b);
FloatExpr operator*(FloatExpr a, FloatExpr b);
FloatExpr operator/(FloatExpr a, FloatExpr b);
FloatExpr min(FloatExpr a, FloatExpr b);
FloatExpr max(FloatExpr a, FloatExpr b);

// SFU functions
FloatExpr recip(FloatExpr x);
FloatExpr recipsqrt(FloatExpr x);
FloatExpr exp(FloatExpr x);
FloatExpr log(FloatExpr x);  

FloatExpr sin(FloatExpr x);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_FLOAT_H_
