#include "Complex.h"
#include "Support/debug.h"

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class Complex
///////////////////////////////////////////////////////////////////////////////

ComplexExpr::ComplexExpr(Complex const &rhs) {
  assertq("Not implemented yet");  // TODO
}


///////////////////////////////////////////////////////////////////////////////
// Class Complex
///////////////////////////////////////////////////////////////////////////////

Complex::Complex(Float re, Float im) : Re(re), Im(im) {}

Complex::Complex(const Complex &rhs) : Re(rhs.Re), Im(rhs.Im) {}

Complex::Complex(ComplexExpr input) {
  assertq("Not implemented yet");  // TODO
}


Complex::Complex(Ptr::Deref d) {
  assertq("Not implemented yet");  // TODO
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


///////////////////////////////////////////////////////////////////////////////
// Class Complex::Ptr
///////////////////////////////////////////////////////////////////////////////

Complex::Ptr::Ptr(ComplexExpr rhs) {
  assertq("Not implemented yet");  // TODO
}


Complex::Ptr::Deref &Complex::Ptr::Deref::operator=(Complex const &rhs) {
  assertq("Not implemented yet");  // TODO
  return *this;
}


// TODO prob not needded, think about it
Complex::Ptr::Deref::Deref(Complex const &rhs) {
  assertq("Not implemented yet");  // TODO
}


Complex::Ptr::Deref Complex::Ptr::operator*() {
  assertq("Not implemented yet");  // TODO
  return Deref(Complex(-1,-1));  // Dummy return
}


Complex::Ptr Complex::Ptr::mkArg() {
  assertq("Not implemented yet");  // TODO

  Expr::Ptr re_e = Pointer::getUniformPtr();
  Expr::Ptr im_e = Pointer::getUniformPtr();

  Complex::Ptr x = ComplexExpr(re_e, im_e);
  return x;
}


bool Complex::Ptr::passParam(Seq<int32_t> *uniforms, Complex::Array *p) {
  assertq("Not implemented yet");  // TODO
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Complex::Ptr
///////////////////////////////////////////////////////////////////////////////

Complex::Array::ref &Complex::Array::ref::operator=(complex const &rhs) {
  assertq("Not implemented yet");  // TODO
  return *this;
}


bool Complex::Array::ref::operator==(complex const &rhs) const {
  assertq("Not implemented yet");  // TODO
  return true;
}


Complex::Array::Array(int size) {
  assertq("Not implemented yet");  // TODO
}


void Complex::Array::dump() const {
  assertq("Not implemented yet");  // TODO
}


void Complex::Array::fill(complex const &rhs) {
  assertq("Not implemented yet");  // TODO
}


Complex::Array::ref Complex::Array::operator[] (int i) {
  assertq("Not implemented yet");  // TODO
  return ref();  // Dummy return
}

}  // namespace V3DLib
