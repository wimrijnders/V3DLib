#ifndef _V3DLIB_SOURCE_COND_H_
#define _V3DLIB_SOURCE_COND_H_

#include "Source/Int.h"

namespace V3DLib {

// ============================================================================
// Types                   
// ============================================================================

struct Cond
{
  // Abstract syntax tree
  CExpr* cexpr;
  // Constructor
  Cond(CExpr* c) { cexpr = c; }
};

struct BoolExpr {
  // Abstract syntax tree
  BExpr* bexpr;
  // Constructor
  BoolExpr(BExpr* b) { bexpr = b; }
};


// ============================================================================
// Specific 'Int' comparisons
// ============================================================================

BoolExpr operator==(IntExpr a, IntExpr b);
BoolExpr operator!=(IntExpr a, IntExpr b);
BoolExpr operator<(IntExpr a, IntExpr b);
BoolExpr operator<=(IntExpr a, IntExpr b);
BoolExpr operator>(IntExpr a, IntExpr b);
BoolExpr operator>=(IntExpr a, IntExpr b);


// ============================================================================
// Specific 'Float' comparisons
// ============================================================================

BoolExpr operator==(FloatExpr a, FloatExpr b);
BoolExpr operator!=(FloatExpr a, FloatExpr b);
BoolExpr operator<(FloatExpr a, FloatExpr b);
BoolExpr operator<=(FloatExpr a, FloatExpr b);
BoolExpr operator>(FloatExpr a, FloatExpr b);
BoolExpr operator>=(FloatExpr a, FloatExpr b);


// ============================================================================
// Boolean operators
// ============================================================================

inline BoolExpr operator!(BoolExpr a)              { return BoolExpr(a.bexpr->Not()); }
inline BoolExpr operator&&(BoolExpr a, BoolExpr b) { return BoolExpr(a.bexpr->And(b.bexpr)); }
inline BoolExpr operator||(BoolExpr a, BoolExpr b) { return BoolExpr(a.bexpr->Or(b.bexpr)); }

inline Cond any(BoolExpr a) { return Cond(mkAny(a.bexpr)); }
inline Cond all(BoolExpr a) { return Cond(mkAll(a.bexpr)); }

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_COND_H_
