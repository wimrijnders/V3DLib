// This module defines type 'Int' for a vector of 16 x 32-bit integers.

#ifndef _V3DLIB_SOURCE_INT_H_
#define _V3DLIB_SOURCE_INT_H_

#include <assert.h>
#include "Source/Syntax.h"
#include "Source/Float.h"

namespace V3DLib {

// ============================================================================
// Types                   
// ============================================================================

// An 'IntExpr' defines an integer vector expression which can
// only be used on the RHS of assignment statements.

struct IntExpr : public BaseExpr {
  IntExpr(int x);
  IntExpr(ExprPtr e) : BaseExpr(e) {}
};

// An 'Int' defines an integer vector variable which can be used in
// both the LHS and RHS of an assignment.

struct Int : public BaseExpr {
  Int();
  Int(int x);
  Int(IntExpr e);

  // Copy constructors
  Int(Int& x);
  Int(const Int& x);

  // Cast to an IntExpr
  operator IntExpr();

  Int& operator=(Int& rhs);
  IntExpr operator=(IntExpr rhs);

  void operator++(int);
};


// ============================================================================
// Operations
// ============================================================================

IntExpr getUniformInt();
IntExpr index();
IntExpr me();
IntExpr numQPUs();
IntExpr vpmGetInt();

IntExpr rotate(IntExpr a, IntExpr b);
FloatExpr rotate(FloatExpr a, IntExpr b);

IntExpr operator+(IntExpr a, IntExpr b);
IntExpr operator-(IntExpr a, IntExpr b);
IntExpr operator*(IntExpr a, IntExpr b);
IntExpr min(IntExpr a, IntExpr b);
IntExpr max(IntExpr a, IntExpr b);
IntExpr operator<<(IntExpr a, IntExpr b);
IntExpr operator>>(IntExpr a, IntExpr b);
IntExpr operator&(IntExpr a, IntExpr b);
IntExpr operator|(IntExpr a, IntExpr b);
IntExpr operator^(IntExpr a, IntExpr b);
IntExpr operator~(IntExpr a);
IntExpr shr(IntExpr a, IntExpr b);
IntExpr ror(IntExpr a, IntExpr b);
IntExpr toInt(FloatExpr a);
FloatExpr toFloat(IntExpr a);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_INT_H_
