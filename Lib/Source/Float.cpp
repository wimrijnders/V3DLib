#include "Source/Float.h"
#include "Lang.h"  // only for assign()!

namespace V3DLib {

// ============================================================================
// Type 'FloatExpr'
// ============================================================================

FloatExpr::FloatExpr(float x) {
	m_expr = std::make_shared<Expr>(x);
}

FloatExpr::FloatExpr(Deref<Float> d) : BaseExpr(d.expr()) {}


// ============================================================================
// Type 'Float'
// ============================================================================

Float::Float() {
  Var v  = freshVar();
  m_expr = mkVar(v);
}


Float::Float(float x) {
  Var v  = freshVar();
  m_expr = mkVar(v);
	auto a = std::make_shared<Expr>(x);
  assign(m_expr, a);
}


Float::Float(FloatExpr e) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, e.expr());
}


Float::Float(Expr::Ptr e, bool set_direct) {
	assert(set_direct == true);
	m_expr = e;
}


// Copy constructors

Float::Float(Float &x) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, x.expr());
}

Float::Float(Float const &x) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, x.expr());
}


// Cast to an FloatExpr

Float::operator FloatExpr() { return FloatExpr(m_expr); }

// Assignment

Float& Float::operator=(Float &rhs) {
	assign(m_expr, rhs.expr());
	return rhs;
}


FloatExpr Float::operator=(FloatExpr rhs) {
	assign(m_expr, rhs.expr());
	return rhs;
}

// ============================================================================
// Generic operations
// ============================================================================

inline FloatExpr mkFloatApply(FloatExpr a,Op op,FloatExpr b) {
  Expr::Ptr e = mkApply(a.expr(), op, b.expr());
  return FloatExpr(e);
}

// ============================================================================
// Specific operations
// ============================================================================

// Read an Float from the UNIFORM FIFO.
FloatExpr getUniformFloat() {
  Expr::Ptr e = mkVar(UNIFORM);
  return FloatExpr(e);
}


// Read vector from VPM
FloatExpr vpmGetFloat() {
  Expr::Ptr e = mkVar(VPM_READ);
  return FloatExpr(e);
}


FloatExpr operator+(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(ADD, FLOAT), b); }
FloatExpr operator-(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(SUB, FLOAT), b); }
FloatExpr operator*(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(MUL, FLOAT), b); }
FloatExpr min(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MIN, FLOAT), b); }
FloatExpr max(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MAX, FLOAT), b); }

}  // namespace V3DLib
