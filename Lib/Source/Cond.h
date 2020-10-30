#ifndef _QPULIB_SOURCE_COND_H_
#define _QPULIB_SOURCE_COND_H_

#include "Source/Syntax.h"
#include "Source/Int.h"

namespace QPULib {

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

struct BoolExpr
{
  // Abstract syntax tree
  BExpr* bexpr;
  // Constructor
  BoolExpr(BExpr* b) { bexpr = b; }
  // Cast to Cond
  //operator Cond();
};

// ============================================================================
// Generic 'Int' comparison
// ============================================================================

inline BoolExpr mkIntCmp(IntExpr a, CmpOp op, IntExpr b)
  { return BoolExpr(mkCmp(a.expr, op, b.expr)); }

// ============================================================================
// Specific 'Int' comparisons
// ============================================================================

inline BoolExpr operator==(IntExpr a, IntExpr b)
  { return mkIntCmp(a, CmpOp(EQ, INT32), b); }

inline BoolExpr operator!=(IntExpr a, IntExpr b)
  { return mkIntCmp(a, CmpOp(NEQ, INT32), b); }

inline BoolExpr operator<(IntExpr a, IntExpr b)
  { return mkIntCmp(a, CmpOp(LT, INT32), b); }

inline BoolExpr operator<=(IntExpr a, IntExpr b)
  { return mkIntCmp(a, CmpOp(LE, INT32), b); }

inline BoolExpr operator>(IntExpr a, IntExpr b)
  { return mkIntCmp(a, CmpOp(GT, INT32), b); }

inline BoolExpr operator>=(IntExpr a, IntExpr b)
  { return mkIntCmp(a, CmpOp(GE, INT32), b); }

// ============================================================================
// Generic 'Float' comparison
// ============================================================================

inline BoolExpr mkFloatCmp(FloatExpr a, CmpOp op, FloatExpr b)
  { return BoolExpr(mkCmp(a.expr, op, b.expr)); }

// ============================================================================
// Specific 'Float' comparisons
// ============================================================================

inline BoolExpr operator==(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, CmpOp(EQ, FLOAT), b); }

inline BoolExpr operator!=(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, CmpOp(NEQ, FLOAT), b); }

inline BoolExpr operator<(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, CmpOp(LT, FLOAT), b); }

inline BoolExpr operator<=(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, CmpOp(LE, FLOAT), b); }

inline BoolExpr operator>(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, CmpOp(GT, FLOAT), b); }

inline BoolExpr operator>=(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, CmpOp(GE, FLOAT), b); }

// ============================================================================
// Boolean operators
// ============================================================================

inline BoolExpr operator!(BoolExpr a)
  { return BoolExpr(mkNot(a.bexpr)); }

inline BoolExpr operator&&(BoolExpr a, BoolExpr b)
  { return BoolExpr(mkAnd(a.bexpr, b.bexpr)); }

inline BoolExpr operator||(BoolExpr a, BoolExpr b)
  { return BoolExpr(mkOr(a.bexpr, b.bexpr)); }

inline Cond any(BoolExpr a)
  { return Cond(mkAny(a.bexpr)); }

inline Cond all(BoolExpr a)
  { return Cond(mkAll(a.bexpr)); }

}  // namespace QPULib

#endif  // _QPULIB_SOURCE_COND_H_
