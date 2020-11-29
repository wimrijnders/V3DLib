#include "Int.h"
#include "Lang.h"  // only for assign()!
#include "SourceTranslate.h"
#include "Support/Platform.h"

namespace V3DLib {

// ============================================================================
// Type 'IntExpr'
// ============================================================================

IntExpr::IntExpr(int x) { m_expr = mkIntLit(x); }

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


Int::Int(Expr::Ptr e, bool set_direct) {
	assert(set_direct == true);
	m_expr = e;
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

Int::operator IntExpr() { return IntExpr(m_expr); }

// Assignment

Int &Int::operator=(Int &rhs) {
	assign(m_expr, rhs.expr());
	return rhs;
}

IntExpr Int::operator=(IntExpr rhs) {
	assign(m_expr, rhs.expr());
	return rhs;
};

// ============================================================================
// Generic operations
// ============================================================================

inline IntExpr mkIntApply(IntExpr a, Op op, IntExpr b)
{
  Expr::Ptr e = mkApply(a.expr(), op, b.expr());
  return IntExpr(e);
}

// ============================================================================
// Specific operations
// ============================================================================

// Read an Int from the UNIFORM FIFO.
IntExpr getUniformInt() {
 	Expr::Ptr e = std::make_shared<Expr>(Var(UNIFORM));
  return IntExpr(e);
}


/**
 * A vector containing integers 0..15
 *
 * On `vc4` this is a special register, on `v3d` this is an instruction.
 */
IntExpr index() {
	if (Platform::instance().compiling_for_vc4()) {
  	Expr::Ptr e = std::make_shared<Expr>(Var(ELEM_NUM));
	  return IntExpr(e);
	} else {
		Expr::Ptr a = mkVar(Var(DUMMY));
  	Expr::Ptr e = mkApply(a, Op(EIDX, INT32), a);
  	return IntExpr(e);
	}
}

// A vector containing the QPU id
IntExpr me() {
  // There is reserved var holding the QPU ID.
  Expr::Ptr e = std::make_shared<Expr>(Var(STANDARD, RSV_QPU_ID));
  return IntExpr(e);
}


// A vector containing the QPU count
IntExpr numQPUs() {
  // There is reserved var holding the QPU count.
  Expr::Ptr e = std::make_shared<Expr>(Var(STANDARD, RSV_NUM_QPUS));
  return IntExpr(e);
}


// Read vector from VPM
IntExpr vpmGetInt() {
  Expr::Ptr e = std::make_shared<Expr>(Var(VPM_READ));
  return IntExpr(e);
}


// Vector rotation
IntExpr rotate(IntExpr a, IntExpr b)
  { return mkIntApply(a, Op(ROTATE, INT32), b); }


FloatExpr rotate(FloatExpr a, IntExpr b) {
  Expr::Ptr e = mkApply(a.expr(), Op(ROTATE, FLOAT), b.expr());
  return FloatExpr(e);
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
  Expr::Ptr e = mkApply(a.expr(), Op(FtoI, INT32), mkIntLit(0));
  return IntExpr(e);
}


// Conversion to Float
FloatExpr toFloat(IntExpr a) {
  Expr::Ptr e = mkApply(a.expr(), Op(ItoF, FLOAT), mkIntLit(0));
  return FloatExpr(e);
}

}  // namespace V3DLib
