///////////////////////////////////////////////////////////////////////////////
// This module defines type 'Int' for a vector of 16 x 32-bit integers.
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_SOURCE_INT_H_
#define _V3DLIB_SOURCE_INT_H_
#include "Common/SharedArray.h"
#include "Common/Seq.h"
#include "Ptr.h"
#include "Expr.h"

namespace V3DLib {

// Reserved general-purpose vars
enum ReservedVarId : VarId {
  RSV_QPU_ID   = 0,
  RSV_NUM_QPUS = 1
};

template <typename T> struct Deref; // Forward declaration template class


// ============================================================================
// Class Int                   
// ============================================================================

/**
 * An 'Int' defines an integer vector variable which can be used in
 * both the LHS and RHS of an assignment.
 */
struct Int : public BaseExpr {
  using Array = V3DLib::SharedArray<int>;
  using Ptr   = V3DLib::ptr::Ptr<Int>;

  Int();
  Int(int x);
  Int(IntExpr e);
  Int(Deref<Int> d);
  Int(Int const &x);

  static Int mkArg();
  static bool passParam(IntList &uniforms, int val);

  operator IntExpr() const;

  Int& operator=(int x);
  Int& operator=(Int const &rhs);
  IntExpr operator=(IntExpr rhs);

  void operator++(int);
  Int &operator+=(IntExpr rhs);
  Int &operator-=(IntExpr rhs);
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

IntExpr operator+(IntExpr a, IntExpr b);
IntExpr operator-(IntExpr a, IntExpr b);
IntExpr operator*(IntExpr a, IntExpr b) ;
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

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_INT_H_
