#include "Cond.h"

namespace V3DLib {
namespace {

/**
 * Generic 'Float' comparison
 */
BoolExpr mkFloatCmp(FloatExpr a, CmpOp::Id cond, FloatExpr b) {
	return BoolExpr(mkCmp(a.expr(), CmpOp(cond, FLOAT), b.expr()));
}


/**
 * Generic 'Int' comparison
 */
BoolExpr mkIntCmp(IntExpr a, CmpOp::Id cond, IntExpr b) {
	return BoolExpr(mkCmp(a.expr(), CmpOp(cond, INT32), b.expr()));
}

}  // anon namespace


BExpr::Ptr mkCmp(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs) {
  return BExpr::Ptr(new BExpr(lhs, op, rhs));
}


// ============================================================================
// Specific 'Int' comparisons
// ============================================================================

BoolExpr operator==(IntExpr a, IntExpr b) { return mkIntCmp(a, CmpOp::EQ,  b); }
BoolExpr operator!=(IntExpr a, IntExpr b) { return mkIntCmp(a, CmpOp::NEQ, b); }
BoolExpr operator<(IntExpr a, IntExpr b)  { return mkIntCmp(a, CmpOp::LT,  b); }
BoolExpr operator<=(IntExpr a, IntExpr b) { return mkIntCmp(a, CmpOp::LE,  b); }
BoolExpr operator>(IntExpr a, IntExpr b)  { return mkIntCmp(a, CmpOp::GT,  b); }
BoolExpr operator>=(IntExpr a, IntExpr b) { return mkIntCmp(a, CmpOp::GE,  b); }


// ============================================================================
// Specific 'Float' comparisons
// ============================================================================

BoolExpr operator==(FloatExpr a, FloatExpr b) { return mkFloatCmp(a, CmpOp::EQ,  b); }
BoolExpr operator!=(FloatExpr a, FloatExpr b) { return mkFloatCmp(a, CmpOp::NEQ, b); }
BoolExpr operator<(FloatExpr a, FloatExpr b)  { return mkFloatCmp(a, CmpOp::LT,  b); }
BoolExpr operator<=(FloatExpr a, FloatExpr b) { return mkFloatCmp(a, CmpOp::LE,  b); }
BoolExpr operator>(FloatExpr a, FloatExpr b)  { return mkFloatCmp(a, CmpOp::GT,  b); }
BoolExpr operator>=(FloatExpr a, FloatExpr b) { return mkFloatCmp(a, CmpOp::GE,  b); }


// ============================================================================
// Boolean operators
// ============================================================================

BoolExpr operator!(BoolExpr a)              { return BoolExpr(a.bexpr()->Not()); }
BoolExpr operator&&(BoolExpr a, BoolExpr b) { return BoolExpr(a.bexpr()->And(b.bexpr())); }
BoolExpr operator||(BoolExpr a, BoolExpr b) { return BoolExpr(a.bexpr()->Or(b.bexpr())); }

Cond any(BoolExpr a) { return Cond(mkAny(a.bexpr())); }
Cond all(BoolExpr a) { return Cond(mkAll(a.bexpr())); }

}  // namespace V3DLib

