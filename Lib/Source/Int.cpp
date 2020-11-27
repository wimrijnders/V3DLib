#include "Source/Int.h"
#include "Source/Stmt.h"
#include "Source/Float.h"
#include "SourceTranslate.h"
#include "Support/Platform.h"

namespace V3DLib {

// ============================================================================
// Type 'IntExpr'
// ============================================================================

IntExpr::IntExpr(int x) { m_expr = mkIntLit(x); }

// Helper constructor
// TODO remove
inline IntExpr mkIntExpr(Expr* e) { return IntExpr(e); }

// ============================================================================
// Type 'Int'
// ============================================================================

// Constructors

Int::Int() {
  Var v    = freshVar();
  m_expr = mkVar(v);
}

Int::Int(int x) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, mkIntLit(x));
}

Int::Int(IntExpr e) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, e.expr());
}

// Copy constructors

Int::Int(Int &x) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, x.expr());
}


Int::Int(Int const &x) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, x.expr());
}


// Cast to an IntExpr

Int::operator IntExpr() { return mkIntExpr(m_expr); }

// Assignment

Int& Int::operator=(Int &rhs)
  { assign(m_expr, rhs.expr()); return rhs; }

IntExpr Int::operator=(IntExpr rhs)
  { assign(m_expr, rhs.expr()); return rhs; };

// ============================================================================
// Generic operations
// ============================================================================

inline IntExpr mkIntApply(IntExpr a, Op op, IntExpr b)
{
  Expr* e = mkApply(a.expr(), op, b.expr());
  return mkIntExpr(e);
}

// ============================================================================
// Specific operations
// ============================================================================

// Read an Int from the UNIFORM FIFO.
IntExpr getUniformInt() {
  Expr* e    = new Expr(Var(UNIFORM));
  return mkIntExpr(e);
}


/**
 * A vector containing integers 0..15
 *
 * On `vc4` this is a special register, on `v3d` this is an instruction.
 */
IntExpr index() {
	if (Platform::instance().compiling_for_vc4()) {
  	Expr* e    = new Expr(Var(ELEM_NUM));
	  return mkIntExpr(e);
	} else {
		Var dummy(DUMMY);

		Expr *a = mkVar(dummy);
  	Expr *e =  mkApply(a, Op(EIDX, INT32), a);
  	return mkIntExpr(e);
	}
}

// A vector containing the QPU id
IntExpr me() {
  // There is reserved var holding the QPU ID.
  Expr* e    = new Expr(Var(STANDARD, RSV_QPU_ID));
  return mkIntExpr(e);
}


// A vector containing the QPU count
IntExpr numQPUs() {
  // There is reserved var holding the QPU count.
  Expr* e    = new Expr(Var(STANDARD, RSV_NUM_QPUS));
  return mkIntExpr(e);
}


// Read vector from VPM
IntExpr vpmGetInt() {
  Expr* e    = new Expr(Var(VPM_READ));
  return mkIntExpr(e);
}


// Vector rotation
IntExpr rotate(IntExpr a, IntExpr b)
  { return mkIntApply(a, Op(ROTATE, INT32), b); }


FloatExpr rotate(FloatExpr a, IntExpr b) {
  Expr* e = mkApply(a.expr(), Op(ROTATE, FLOAT), b.expr());
  return mkFloatExpr(e);
}

void Int::operator++(int) { *this = *this + 1; }

IntExpr operator+(IntExpr a, IntExpr b) { return mkIntApply(a, Op(ADD, INT32), b); }
IntExpr operator-(IntExpr a, IntExpr b) { return mkIntApply(a, Op(SUB, INT32), b); }
IntExpr operator*(IntExpr a, IntExpr b) { return mkIntApply(a, Op(MUL, INT32), b); }
IntExpr min(IntExpr a, IntExpr b) { return mkIntApply(a, Op(MIN, INT32), b); }
IntExpr max(IntExpr a, IntExpr b) { return mkIntApply(a, Op(MAX, INT32), b); }
IntExpr operator<<(IntExpr a, IntExpr b) { return mkIntApply(a, Op(SHL, INT32), b); }
IntExpr operator>>(IntExpr a, IntExpr b) { return mkIntApply(a, Op(SHR, INT32), b); }
IntExpr operator&(IntExpr a, IntExpr b) { return mkIntApply(a, Op(BAND, INT32), b); }
IntExpr operator|(IntExpr a, IntExpr b) { return mkIntApply(a, Op(BOR, INT32), b); }
IntExpr operator^(IntExpr a, IntExpr b) { return mkIntApply(a, Op(BXOR, INT32), b); }
IntExpr operator~(IntExpr a) { return mkIntApply(a, Op(BNOT, INT32), a); }
IntExpr shr(IntExpr a, IntExpr b) { return mkIntApply(a, Op(USHR, INT32), b); }
IntExpr ror(IntExpr a, IntExpr b) { return mkIntApply(a, Op(ROR, INT32), b); }

// Conversion to Int
IntExpr toInt(FloatExpr a) {
  Expr* e = mkApply(a.expr(), Op(FtoI, INT32), mkIntLit(0));
  return mkIntExpr(e);
}

// Conversion to Float
FloatExpr toFloat(IntExpr a) {
  Expr* e = mkApply(a.expr(), Op(ItoF, FLOAT), mkIntLit(0));
  return mkFloatExpr(e);
}

}  // namespace V3DLib
