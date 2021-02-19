#include "Complex.h"
#include "Support/basics.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

///////////////////////////////////////////////////////////////////////////////
// Class ComplexExpr
///////////////////////////////////////////////////////////////////////////////

ComplexExpr::ComplexExpr(Complex const &rhs) {
  assertq("Not implemented yet", true);  // TODO
}


///////////////////////////////////////////////////////////////////////////////
// Class complex
///////////////////////////////////////////////////////////////////////////////

std::string complex::dump() const {
  std::string ret;

  bool int_only = (m_re == (float) ((int) m_re)) && (m_im == (float) ((int) m_im));

  if (int_only) {
    ret << "(" << (int) m_re << ", " << (int) m_im << ")";
  } else {
    ret << "(" << m_re << ", " << m_im << ")";
  }

  return ret;
}



///////////////////////////////////////////////////////////////////////////////
// Class Complex
///////////////////////////////////////////////////////////////////////////////

Complex::Complex(Float re, Float im) : Re(re), Im(im) {}

Complex::Complex(const Complex &rhs) : Re(rhs.Re), Im(rhs.Im) {}

Complex::Complex(ComplexExpr input) {
  assertq("Not implemented yet", true);  // TODO
}


Complex::Complex(Ptr::Deref d) {
  Re = Float(d.m_re);
  Im = Float(d.m_im);
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
  re = rhs.re();
  im = rhs.im();
}


Complex::Ptr::Deref::Deref(Expr::Ptr re, Expr::Ptr im) : m_re(re), m_im(im) {}


/**
 * NOTE: return value differen from `Deref<T>`, which return `T [const] &` for rhs
 */
Complex::Ptr::Deref &Complex::Ptr::Deref::operator=(Complex const &rhs) {
  m_re = rhs.Re;
  m_im = rhs.Im;
  return *this;
}


Complex::Ptr::Deref Complex::Ptr::operator*() {
  auto re_deref = mkDeref(re.expr());
  auto im_deref = mkDeref(im.expr());

  return Deref(re_deref, im_deref);
}


Complex::Ptr Complex::Ptr::mkArg() {
  Expr::Ptr re_e = Pointer::getUniformPtr();
  Expr::Ptr im_e = Pointer::getUniformPtr();

  Complex::Ptr x = ComplexExpr(re_e, im_e);
  return x;
}


bool Complex::Ptr::passParam(Seq<int32_t> *uniforms, Complex::Array *p) {
  uniforms->append(p->re().getAddress());
  uniforms->append(p->im().getAddress());
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Complex::Array
///////////////////////////////////////////////////////////////////////////////

Complex::Array::ref &Complex::Array::ref::operator=(complex const &rhs) {
  m_re_ref = rhs.re();
  m_im_ref = rhs.im();
  return *this;
}


std::string Complex::Array::ref::dump() const {
  auto tmp = complex(m_re_ref, m_im_ref);
  return tmp.dump();
}


bool Complex::Array::ref::operator==(complex const &rhs) const {
  return (m_re_ref == rhs.re() && m_im_ref == rhs.im());
}


Complex::Array::Array(int size) : m_re(size), m_im(size) {}


std::string Complex::Array::dump() const {
  assert(m_re.size() == m_im.size());
  std::string ret;

  bool int_only = no_fractions(m_re) && no_fractions(m_im);

  for (int i = 0; i < (int) m_re.size(); ++i) {
    if ( i != 0 && i % 16 == 0) {
      ret << "\n";
    }

    if (int_only) {
      ret << "(" << (int) m_re[i] << ", " << (int) m_im[i] << "), ";
    } else {
      ret << "(" << m_re[i] << ", " << m_im[i] << "), ";
    }
  }

  ret << "\n";

  return ret;
}


void Complex::Array::fill(complex const &rhs) {
  m_re.fill(rhs.re());
  m_im.fill(rhs.im());
}


Complex::Array::ref::ref(float &re_ref, float &im_ref) : m_re_ref(re_ref), m_im_ref(im_ref) {}

Complex::Array::ref Complex::Array::operator[] (int i) {
  return ref(m_re[i], m_im[i]);
}

}  // namespace V3DLib
