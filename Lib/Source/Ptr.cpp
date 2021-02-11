#include "Ptr.h"
#include "Lang.h"  // comment()

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

namespace {
std::unique_ptr<Int> increment;

Int &get_increment() {
  if (increment.get() == nullptr) {
    int const INC = 16*4;  // for getting next block for a sequential pointer
    increment.reset(new Int(INC));  comment("pointer increment");
  }

  return *increment;
}

}  // anon namespace


void Pointer::reset_increment() {
  increment.reset(nullptr);
}


Pointer::Pointer() : BaseExpr(mkVar(freshVar()), "Ptr") {}


Pointer &Pointer::me() {
  return *(const_cast<Pointer *>(this));
}


Pointer::Pointer(PointerExpr rhs) : Pointer() {
  assign(expr(), rhs.expr());
}


void Pointer::inc() {
  me() = bare_addself(get_increment());  comment("increment pointer");
}


PointerExpr Pointer::operator+(int b)     { return add(b); }
PointerExpr Pointer::operator+=(Int &b)   { return addself(b); }
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


PointerExpr Pointer::bare_addself(Int &b) {
  return mkApply(expr(), Op(ADD, INT32), b.expr());
}


PointerExpr Pointer::addself(IntExpr b) {
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
