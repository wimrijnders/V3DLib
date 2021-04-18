#include "Int.h"
#include "Lang.h"  // only for assign()!
#include "SourceTranslate.h"
#include "Support/Platform.h"
#include "Support/debug.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

// ============================================================================
// Class Int
// ============================================================================

Int::Int()              { assign_intern(); }
Int::Int(int x)         { assign_intern(mkIntLit(x)); }
Int::Int(Deref<Int> d)  { assign_intern(d.expr()); }
Int::Int(IntExpr e)     { assign_intern(e.expr()); }
Int::Int(Int const &x)  { assign_intern(x.expr());
}


/**
 * Cast to an IntExpr
 */
Int::operator IntExpr() const {
  return IntExpr(m_expr);
}


Int &Int::operator=(int x) {
  Int tmp(x);
  (*this) = tmp;
  return *this; //rhs;
}

Int &Int::operator=(Int const &rhs) {
  assign(m_expr, rhs.expr());
  return *this; //rhs;
}

IntExpr Int::operator=(IntExpr rhs) {
  assign(m_expr, rhs.expr());
  return rhs;
}


Int &Int::operator+=(IntExpr rhs) { *this = *this + rhs; return *this; }
Int &Int::operator-=(IntExpr rhs) { *this = *this - rhs; return *this; }
Int &Int::operator|=(IntExpr rhs) { *this = *this | rhs; return *this; }


//
// Note that these do not follow the c-convention that inc/dec
// is performed AFTER current expression.
//
void Int::operator++(int) { *this = *this + 1; }
void Int::operator--(int)  { *this = *this - 1; }


// ============================================================================
// Generic operations
// ============================================================================

inline IntExpr mkIntApply(IntExpr a, Op const &op, IntExpr b) {
  Expr::Ptr e = mkApply(a.expr(), op, b.expr());
  return IntExpr(e);
}


// ============================================================================
// Specific operations
// ============================================================================

/**
 * Read an Int from the UNIFORM FIFO.
 */
IntExpr getUniformInt() {
   Expr::Ptr e = std::make_shared<Expr>(Var(UNIFORM));
  return IntExpr(e);
}


Int Int::mkArg() {
  Int x;
  x = getUniformInt();
  return x;
}


bool Int::passParam(IntList &uniforms, int val) {
  uniforms.append((int32_t) val);
  return true;
}


/**
 * A vector containing integers 0..15
 *
 * On `vc4` this is a special register, on `v3d` this is an instruction.
 */
IntExpr index() {
  if (Platform::compiling_for_vc4()) {
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


/**
 * Read vector from VPM
 */
IntExpr vpmGetInt() {
  Expr::Ptr e = std::make_shared<Expr>(Var(VPM_READ));
  return IntExpr(e);
}


/**
 * Vector rotation for int values
 */
IntExpr rotate(IntExpr a, IntExpr b) {
  return mkIntApply(a, Op(ROTATE, INT32), b);
}


IntExpr operator+(IntExpr a, IntExpr b)  { return mkIntApply(a, Op(ADD,  INT32), b); }
IntExpr operator-(IntExpr a, IntExpr b)  { return mkIntApply(a, Op(SUB,  INT32), b); }
IntExpr operator*(IntExpr a, IntExpr b)  { return mkIntApply(a, Op(MUL,  INT32), b); }
IntExpr min(IntExpr a, IntExpr b)        { return mkIntApply(a, Op(MIN,  INT32), b); }
IntExpr max(IntExpr a, IntExpr b)        { return mkIntApply(a, Op(MAX,  INT32), b); }
IntExpr operator<<(IntExpr a, IntExpr b) { return mkIntApply(a, Op(SHL,  INT32), b); }
IntExpr operator>>(IntExpr a, IntExpr b) { return mkIntApply(a, Op(SHR,  INT32), b); }
IntExpr operator&(IntExpr a, IntExpr b)  { return mkIntApply(a, Op(BAND, INT32), b); }
IntExpr operator|(IntExpr a, IntExpr b)  { return mkIntApply(a, Op(BOR,  INT32), b); }
IntExpr operator^(IntExpr a, IntExpr b)  { return mkIntApply(a, Op(BXOR, INT32), b); }
IntExpr operator~(IntExpr a)             { return mkIntApply(a, Op(BNOT, INT32), a); }
IntExpr shr(IntExpr a, IntExpr b)        { return mkIntApply(a, Op(USHR, INT32), b); }
IntExpr ror(IntExpr a, IntExpr b)        { return mkIntApply(a, Op(ROR,  INT32), b); }

}  // namespace V3DLib
