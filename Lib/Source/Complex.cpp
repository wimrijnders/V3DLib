#include "Complex.h"
#include "Support/debug.h"

namespace V3DLib {

Complex::Complex(const Complex &rhs) : Re(rhs.Re), Im(rhs.Im) {}

Complex::Complex(PtrExpr<Float> input) {
  assert(false);  // WRONG
  Re = *input;
  Im = *(input + 1);
}


Complex Complex::operator *(Complex rhs) {
  Complex tmp;
  tmp.Re = Re*rhs.Re - Im*rhs.Im;
  tmp.Im = Re*rhs.Im + Im*rhs.Re;

  return tmp;
}


Complex Complex::operator *=(Complex rhs) {
  Complex tmp;

  tmp.Re = Re*rhs.Re - Im*rhs.Im;
  tmp.Im = Re*rhs.Im + Im*rhs.Re;

  return tmp;
}


void Complex::operator=(Complex const &rhs) {
  Re = rhs.Re;
  Im = rhs.Im;
}


Complex::Ptr Complex::Ptr::mkArg() {
  Expr::Ptr e = getUniformPtr();

  Ptr x = PtrExpr<Complex>(e);
  return x;
}

}  // namespace V3DLib
