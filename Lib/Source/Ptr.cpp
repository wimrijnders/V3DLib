#include "Ptr.h"
#include "Common/SharedArray.h"
#include "Lang.h"  // comment()

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class PointerExpr
///////////////////////////////////////////////////////////////////////////////

PointerExpr::PointerExpr(Expr::Ptr e) : BaseExpr(e, "PointerExpr") {}
PointerExpr::PointerExpr(BaseExpr const &e) : BaseExpr(e.expr(), "PointerExpr") {}

PointerExpr &PointerExpr::self() { return *(const_cast<PointerExpr *>(this)); }

PointerExpr PointerExpr::add(IntExpr b) {
  Expr::Ptr e = mkApply(expr(), Op(ADD, INT32), (b << 2).expr());
  return PointerExpr(e);
}


///////////////////////////////////////////////////////////////////////////////
// Class Pointer
///////////////////////////////////////////////////////////////////////////////

namespace {
/*
std::unique_ptr<Int> increment;

Int &get_increment() {
  if (increment.get() == nullptr) {
    int const INC = 16*4;  // for getting next block for a sequential pointer
    increment.reset(new Int(INC));  comment("pointer increment");
  }

  return *increment;
}
*/

}  // anon namespace


Pointer::Pointer() : BaseExpr("Ptr") {}


Pointer::Pointer(PointerExpr rhs) : Pointer() {
  assign(expr(), rhs.expr());
}


PointerExpr Pointer::operator=(PointerExpr rhs) { assign(expr(), rhs.expr()); return rhs; }

PointerExpr Pointer::operator+(int b)       { return add(b); }
PointerExpr Pointer::operator+=(IntExpr b)  { return addself(b); }
PointerExpr Pointer::operator+=(int b)      { return addself(b); }
PointerExpr Pointer::operator+(IntExpr b)   { return add(b); }
PointerExpr Pointer::operator-(IntExpr b)   { return sub(b); }

PointerExpr Pointer::add(int b)     const { return mkApply(expr(), Op(ADD, INT32), mkIntLit(4*b)); }
PointerExpr Pointer::add(IntExpr b) const { return mkApply(expr(), Op(ADD, INT32), (b << 2).expr()); }
PointerExpr Pointer::sub(IntExpr b) const { return mkApply(expr(), Op(SUB, INT32), (b << 2).expr()); }
PointerExpr Pointer::addself(int b)       { return (self() = self() + b); }
PointerExpr Pointer::addself(IntExpr b)   { return (self() = self() + b); }
PointerExpr Pointer::subself(IntExpr b)   { return (self() = self() - b); }

PointerExpr Pointer::bare_addself(IntExpr b) { return mkApply(expr(), Op(ADD, INT32), b.expr()); }


Pointer &Pointer::self() {
  return *(const_cast<Pointer *>(this));
}


void Pointer::reset_increment() {
  // Does nothing for now
  // TODO cleanup
  // increment.reset(nullptr);
}


void Pointer::inc() {
  std::unique_ptr<Int> increment;
  int const INC = 16*4;  // for getting next block for a sequential pointer
  increment.reset(new Int(INC));  comment("pointer increment");

  //self() = bare_addself(get_increment());  comment("increment pointer");
  self() = bare_addself(*increment);  comment("increment pointer");
}


Expr::Ptr Pointer::getUniformPtr() {
  Expr::Ptr e = std::make_shared<Expr>(Var(UNIFORM, true));
  return e;
}


bool Pointer::passParam(IntList &uniforms, BaseSharedArray const *p) {
  uniforms.append(p->getAddress());
  return true;
}

}  // namespace V3DLib
