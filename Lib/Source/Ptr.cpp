#include "Ptr.h"

namespace V3DLib {

PointerExpr::PointerExpr(Expr::Ptr e) : BaseExpr(e, "PointerExpr") {}
PointerExpr::PointerExpr(BaseExpr const &e) : BaseExpr(e.expr(), "PointerExpr") {}

Pointer::Pointer() : BaseExpr(mkVar(freshVar()), "Ptr") {}

Pointer::Pointer(PointerExpr rhs) : Pointer() {
  assign(expr(), rhs.expr());
}


PointerExpr Pointer::operator+(int b) { return add(b); }
PointerExpr Pointer::operator+=(int b) { return addself(b); }
PointerExpr Pointer::operator+(IntExpr b) { return add(b); }


PointerExpr Pointer::operator=(PointerExpr rhs) {
  assign(expr(), rhs.expr());
  return rhs;
}


PointerExpr Pointer::addself(int b) {
  Pointer &me = *(const_cast<Pointer *>(this));
  return (me = me + b);
}


PointerExpr Pointer::add(int b) {
  return mkApply(expr(), Op(ADD, INT32), mkIntLit(4*b));
}


PointerExpr Pointer::add(IntExpr b) {
  breakpoint  // TODO check if ever used and correct working
  return mkApply(expr(), Op(ADD, INT32), (b << 2).expr());
}

}  // namespace V3DLib
