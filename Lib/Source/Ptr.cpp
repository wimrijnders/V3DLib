#include "Ptr.h"

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class PointerExpr
///////////////////////////////////////////////////////////////////////////////

PointerExpr::PointerExpr(Expr::Ptr e) : BaseExpr(e, "PointerExpr") {}
PointerExpr::PointerExpr(BaseExpr const &e) : BaseExpr(e.expr(), "PointerExpr") {}

PointerExpr &PointerExpr::me() {
  return *(const_cast<PointerExpr *>(this));
}

PointerExpr PointerExpr::add(IntExpr b) {
  Expr::Ptr e = mkApply(expr(), Op(ADD, INT32), (b << 2).expr());
  return PointerExpr(e);
}


///////////////////////////////////////////////////////////////////////////////
// Class Pointer
///////////////////////////////////////////////////////////////////////////////

Pointer::Pointer() : BaseExpr(mkVar(freshVar()), "Ptr") {}

Pointer &Pointer::me() {
  return *(const_cast<Pointer *>(this));
}

Pointer::Pointer(PointerExpr rhs) : Pointer() {
  assign(expr(), rhs.expr());
}


PointerExpr Pointer::operator+(int b)     { return add(b); }
PointerExpr Pointer::operator+=(int b)    { return addself(b); }
PointerExpr Pointer::operator+(IntExpr b) { return add(b); }
PointerExpr Pointer::operator-(IntExpr b) { return sub(b); }


PointerExpr Pointer::operator=(PointerExpr rhs) {
  assign(expr(), rhs.expr());
  return rhs;
}


PointerExpr Pointer::addself(int b) {
  return (me() = me() + b);
}


PointerExpr Pointer::subself(IntExpr b) {
  return (me() = me() - b);
}


PointerExpr Pointer::add(int b) {
  return mkApply(expr(), Op(ADD, INT32), mkIntLit(4*b));
}


PointerExpr Pointer::add(IntExpr b) {
  return mkApply(expr(), Op(ADD, INT32), (b << 2).expr());
}


PointerExpr Pointer::sub(IntExpr b) {
  return mkApply(expr(), Op(SUB, INT32), (b << 2).expr());
}

}  // namespace V3DLib
