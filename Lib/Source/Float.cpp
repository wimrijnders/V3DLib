#include "Source/Float.h"
#include "Lang.h"  // only for assign()!
#include "Functions.h"  // ::set_at()

namespace V3DLib {

// ============================================================================
// Class FloatExpr
// ============================================================================

FloatExpr::FloatExpr(float x) { m_expr = std::make_shared<Expr>(x); }
FloatExpr::FloatExpr(Deref<Float> d) : BaseExpr(d.expr()) {}


// ============================================================================
// Class Float
// ============================================================================

Float::Float()               { assign_intern(); }
Float::Float(FloatExpr e)    { assign_intern(e.expr()); }
Float::Float(Deref<Float> d) { assign_intern(d.expr()); }
Float::Float(Float const &x) { assign_intern(x.expr()); }

Float::Float(float x) {
  auto a = std::make_shared<Expr>(x);
  assign_intern(a);
}


bool Float::passParam(Seq<int32_t> *uniforms, float val) {
  int32_t* bits = (int32_t*) &val;
  uniforms->append(*bits);
  return true;
}


/**
 * Reinterpret the incoming integer expression as a float
 */
void Float::as_float(IntExpr rhs) {
  (*this) = FloatExpr(rhs.expr());
}


/**
 * Cast to a FloatExpr
 */
Float::operator FloatExpr() const { return FloatExpr(m_expr); }


/**
 * Assignment
 */

Float &Float::operator=(float rhs) {
  Float tmp(rhs);
  assign(m_expr, tmp.expr());
  return self();
}


Float &Float::operator=(Float &rhs) {
  assign(m_expr, rhs.expr());
  return rhs;
}

Float &Float::operator=(Float const &rhs) {
  assign(m_expr, rhs.expr());
  return self();
}


FloatExpr Float::operator=(FloatExpr const &rhs) {
  assign(m_expr, rhs.expr());
  return self();
}


Float &Float::operator=(Deref<Float> d) {
  Float tmp = d;
  assign(m_expr, tmp.expr());
  return self();
}


Float &Float::self() {
  return *(const_cast<Float *>(this));
}


Float &Float::operator+=(FloatExpr rhs) { *this = *this + rhs; return *this; }
Float &Float::operator-=(FloatExpr rhs) { *this = *this - rhs; return *this; }
Float &Float::operator*=(FloatExpr rhs) { *this = *this * rhs; return *this; }



// ============================================================================
// Generic operations
// ============================================================================

inline FloatExpr mkFloatApply(FloatExpr lhs, Op op, FloatExpr rhs) {
  Expr::Ptr e = mkApply(lhs.expr(), op, rhs.expr());
  return FloatExpr(e);
}


inline FloatExpr mkFloatApply(FloatExpr rhs, Op op) {
  Expr::Ptr e = mkApply(rhs.expr(), op);
  return FloatExpr(e);
}


/**
 * Read an Float from the UNIFORM FIFO.
 */
Float Float::mkArg() {
  Expr::Ptr e = mkVar(UNIFORM);
  Float x;
  x = FloatExpr(e);
  return x;
}


void Float::set_at(Int n, Float const &src) {
  V3DLib::set_at(*this, n, src);
}

// ============================================================================
// Operations
// ============================================================================

// Read vector from VPM
FloatExpr vpmGetFloat() {
  Expr::Ptr e = mkVar(VPM_READ);
  return FloatExpr(e);
}


/**
 * Vector rotation for float values
 */
FloatExpr rotate(FloatExpr a, IntExpr b) {
  Expr::Ptr e = mkApply(a.expr(), Op(ROTATE, FLOAT), b.expr());
  return FloatExpr(e);
}


/**
 * Conversion to Int
 */
IntExpr toInt(FloatExpr a) {
  Expr::Ptr e = mkApply(a.expr(), Op(FtoI, INT32));
  return IntExpr(e);
}


/**
 * Conversion to Float
 */
FloatExpr toFloat(IntExpr a) {
  Expr::Ptr e = mkApply(a.expr(), Op(ItoF, FLOAT));
  return FloatExpr(e);
}


FloatExpr ffloor(FloatExpr a) {
  Expr::Ptr dummy = mkVar(Var(DUMMY));
  Expr::Ptr e = mkApply(a.expr(), Op(FFLOOR, FLOAT), dummy);
  return FloatExpr(e);
}


FloatExpr operator+(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(ADD, FLOAT), b); }
FloatExpr operator-(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(SUB, FLOAT), b); }
FloatExpr operator*(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(MUL, FLOAT), b); }

FloatExpr min(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MIN, FLOAT), b); }
FloatExpr max(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MAX, FLOAT), b); }

// SFU functions
FloatExpr recip(FloatExpr x)     { return mkFloatApply(x, Op(RECIP, FLOAT)); }
FloatExpr recipsqrt(FloatExpr x) { return mkFloatApply(x, Op(RECIPSQRT, FLOAT)); }
FloatExpr exp(FloatExpr x)       { return mkFloatApply(x, Op(EXP, FLOAT)); }
FloatExpr log(FloatExpr x)       { return mkFloatApply(x, Op(LOG, FLOAT)); }

}  // namespace V3DLib
