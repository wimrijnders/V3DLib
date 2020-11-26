#include "Source/Float.h"
#include "Source/Stmt.h"

namespace V3DLib {

// ============================================================================
// Type 'FloatExpr'
// ============================================================================

FloatExpr::FloatExpr(float x) {
	expr = mkFloatLit(x);
}


// ============================================================================
// Type 'Float'
// ============================================================================

Float::Float() {
  Var v    = freshVar();
  this->expr = mkVar(v);
}


Float::Float(float x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, mkFloatLit(x));
}


Float::Float(FloatExpr e) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, e.expr);
}


// Copy constructors

Float::Float(Float& x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, x.expr);
}

Float::Float(const Float& x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, x.expr);
}


// Cast to an FloatExpr

Float::operator FloatExpr() { return mkFloatExpr(this->expr); }

// Assignment

Float& Float::operator=(Float& rhs)
  { assign(this->expr, rhs.expr); return rhs; }

FloatExpr Float::operator=(FloatExpr rhs)
  { assign(this->expr, rhs.expr); return rhs; };

// ============================================================================
// Generic operations
// ============================================================================

inline FloatExpr mkFloatApply(FloatExpr a,Op op,FloatExpr b) {
  Expr* e = mkApply(a.expr, op, b.expr);
  return mkFloatExpr(e);
}

// ============================================================================
// Specific operations
// ============================================================================

// Read an Float from the UNIFORM FIFO.
FloatExpr getUniformFloat() {
  Expr* e    = new Expr;
  e->tag     = VAR;
  e->var.tag = UNIFORM;
  return mkFloatExpr(e);
}

// Read vector from VPM
FloatExpr vpmGetFloat() {
  Expr* e    = new Expr;
  e->tag     = VAR;
  e->var.tag = VPM_READ;
  return mkFloatExpr(e);
}

FloatExpr operator+(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(ADD, FLOAT), b); }
FloatExpr operator-(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(SUB, FLOAT), b); }
FloatExpr operator*(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(MUL, FLOAT), b); }
FloatExpr min(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MIN, FLOAT), b); }
FloatExpr max(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MAX, FLOAT), b); }

}  // namespace V3DLib
