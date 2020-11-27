#include "Cond.h"

namespace V3DLib {
namespace {

BExpr *mkCmp(Expr::Ptr lhs, CmpOp op, Expr::Ptr rhs) {
  return new BExpr(lhs, op, rhs);
}


/**
 * Generic 'Float' comparison
 */
BoolExpr mkFloatCmp(FloatExpr a, CmpOp op, FloatExpr b) {
	return BoolExpr(mkCmp(a.expr(), op, b.expr()));
}


/**
 * Generic 'Int' comparison
 */
BoolExpr mkIntCmp(IntExpr a, CmpOp op, IntExpr b) {
	return BoolExpr(mkCmp(a.expr(), op, b.expr()));
}

}  // anon namespace


// ============================================================================
// Specific 'Int' comparisons
// ============================================================================

BoolExpr operator==(IntExpr a, IntExpr b) { return mkIntCmp(a, CmpOp(EQ, INT32), b); }
BoolExpr operator!=(IntExpr a, IntExpr b) { return mkIntCmp(a, CmpOp(NEQ, INT32), b); }
BoolExpr operator<(IntExpr a, IntExpr b)  { return mkIntCmp(a, CmpOp(LT, INT32), b); }
BoolExpr operator<=(IntExpr a, IntExpr b) { return mkIntCmp(a, CmpOp(LE, INT32), b); }
BoolExpr operator>(IntExpr a, IntExpr b)  { return mkIntCmp(a, CmpOp(GT, INT32), b); }
BoolExpr operator>=(IntExpr a, IntExpr b) { return mkIntCmp(a, CmpOp(GE, INT32), b); }



// ============================================================================
// Specific 'Float' comparisons
// ============================================================================

BoolExpr operator==(FloatExpr a, FloatExpr b) { return mkFloatCmp(a, CmpOp(EQ, FLOAT), b); }
BoolExpr operator!=(FloatExpr a, FloatExpr b) { return mkFloatCmp(a, CmpOp(NEQ, FLOAT), b); }
BoolExpr operator<(FloatExpr a, FloatExpr b)  { return mkFloatCmp(a, CmpOp(LT, FLOAT), b); }
BoolExpr operator<=(FloatExpr a, FloatExpr b) { return mkFloatCmp(a, CmpOp(LE, FLOAT), b); }
BoolExpr operator>(FloatExpr a, FloatExpr b)  { return mkFloatCmp(a, CmpOp(GT, FLOAT), b); }
BoolExpr operator>=(FloatExpr a, FloatExpr b) { return mkFloatCmp(a, CmpOp(GE, FLOAT), b); }

}  // namespace V3DLib

