#include "Source/Float.h"
#include "Source/Stmt.h"

namespace V3DLib {

// ============================================================================
// Type 'FloatExpr'
// ============================================================================

FloatExpr::FloatExpr(float x) {
	m_expr = std::make_shared<Expr>(x);
}


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

Float::operator FloatExpr() { return mkFloatExpr(m_expr); }

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
  Expr* e = mkApply(a.expr(), op, b.expr());
  return mkFloatExpr(e);
}

// ============================================================================
// Specific operations
// ============================================================================

// Read an Float from the UNIFORM FIFO.
FloatExpr getUniformFloat() {
  ExprPtr e    = new Expr(Var(UNIFORM));
  return mkFloatExpr(e);
}


// Read vector from VPM
FloatExpr vpmGetFloat() {
  ExprPtr e = new Expr(Var(VPM_READ));
  return mkFloatExpr(e);
}


FloatExpr operator+(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(ADD, FLOAT), b); }
FloatExpr operator-(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(SUB, FLOAT), b); }
FloatExpr operator*(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(MUL, FLOAT), b); }
FloatExpr min(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MIN, FLOAT), b); }
FloatExpr max(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MAX, FLOAT), b); }

}  // namespace V3DLib
