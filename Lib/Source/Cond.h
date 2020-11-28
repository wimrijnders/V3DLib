#ifndef _V3DLIB_SOURCE_COND_H_
#define _V3DLIB_SOURCE_COND_H_

#include "Source/Int.h"

namespace V3DLib {

// ============================================================================
// Types                   
// ============================================================================

class CExpr;  //  forward declaration
class BExpr;  //  forward declaration

struct Cond {
  CExpr* cexpr; // Abstract syntax tree

  Cond(CExpr* c) { cexpr = c; }
};


struct BoolExpr {
  BExpr* bexpr;  // Abstract syntax tree

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

BoolExpr operator!(BoolExpr a);
BoolExpr operator&&(BoolExpr a, BoolExpr b);
BoolExpr operator||(BoolExpr a, BoolExpr b);

Cond any(BoolExpr a);
Cond all(BoolExpr a);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_COND_H_
