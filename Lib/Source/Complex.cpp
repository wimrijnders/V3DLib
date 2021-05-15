#include "Complex.h"
#include <cmath>
#include "Support/basics.h"
#include "Functions.h"  // ::set_at()
#include "gather.h"

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

/**
 * Init value with phase
 *
 * Param phase is in units of 2PI.
 * Thus, phase = 0.5 corresponds with PI.
 */
complex::complex(float phase) : complex(cosf(2*((float) M_PI)*phase), sinf(2*((float) M_PI)*phase)) {}


float complex::magnitude() const {
 return (float) sqrt(m_re*m_re + m_im*m_im);  // sqrt here is a cmath function
}


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

Complex::Complex(FloatExpr const &e_re, Float const &e_im) : m_re(e_re), m_im(e_im) {}

Complex::Complex(Complex const &rhs) : m_re(rhs.m_re), m_im(rhs.m_im) {}

Complex::Complex(ComplexExpr input) {
  assertq("Not implemented yet", true);  // TODO
}


Complex::Complex(Ptr::Deref d) {
  m_re = d.m_re;
  m_im = d.m_im;
}


Complex::Complex(complex c) : Complex(c.re(), c.im()) {}


/**
 * Init value with phase
 *
 * Param phase is in units of 2PI.
 * Thus, phase = 0.5 corresponds with PI.
 */
Complex::Complex(float phase) : Complex(complex(phase)) {}


Complex &Complex::self() { return *(const_cast<Complex *>(this)); }


/**
 * Returns square of magnitude
 */
Float Complex::mag_square() const {
  return (m_re*m_re + m_im*m_im); 
}


Complex Complex::operator+(Complex rhs) const {
  return Complex(m_re + rhs.m_re, m_im + rhs.m_im);
}


Complex Complex::operator-(Complex rhs) const {
  return Complex(m_re - rhs.m_re, m_im - rhs.m_im);
}


Complex &Complex::operator+=(Complex rhs) {
  m_re += rhs.m_re;
  m_im += rhs.m_im;
  return *this;
}


Complex Complex::operator*(Complex rhs) const {
  Complex tmp;
  tmp.m_re = m_re*rhs.m_re - m_im*rhs.m_im;
  tmp.m_im = m_re*rhs.m_im + m_im*rhs.m_re;

  return tmp;
}


Complex &Complex::operator*=(Complex rhs) {
  *this = (*this)*rhs;
  return *this;
}


void Complex::operator=(Complex const &rhs) {
  m_re = rhs.m_re;
  m_im = rhs.m_im;
}


void Complex::set_at(Int n, Complex const &src) {
  V3DLib::set_at(m_re, n, src.m_re);
  V3DLib::set_at(m_im, n, src.m_im);
}


///////////////////////////////////////////////////////////////////////////////
// Class Complex::Ptr
///////////////////////////////////////////////////////////////////////////////

// TODO prob wrong, get rid of this
Complex::Ptr::Ptr(ComplexExpr rhs) {
  m_re = rhs.re();
  m_im = rhs.im();
}


Complex::Ptr::Ptr(Ptr::Expr rhs) : m_re(rhs.m_re), m_im(rhs.m_im) {}


Complex::Ptr::Deref::Deref(V3DLib::Expr::Ptr re, V3DLib::Expr::Ptr im) : m_re(re), m_im(im) {}


/**
 * NOTE: return value differen from `Deref<T>`, which return `T [const] &` for rhs
 */
Complex::Ptr::Deref &Complex::Ptr::Deref::operator=(Complex const &rhs) {
  m_re = rhs.m_re;
  m_im = rhs.m_im;
  return *this;
}


Complex::Ptr::Deref Complex::Ptr::operator*() {
  auto re_deref = mkDeref(m_re.expr());
  auto im_deref = mkDeref(m_im.expr());

  return Deref(re_deref, im_deref);
}


Complex::Ptr::Deref Complex::Ptr::operator[](IntExpr index) {
  return Deref(m_re[index], m_im[index]);
}


Complex::Ptr &Complex::Ptr::operator+=(IntExpr rhs) {
  m_re += rhs;
  m_im += rhs;
  return *this;
}


Complex::Ptr &Complex::Ptr::operator-=(IntExpr rhs) {
  m_re -= rhs;
  m_im -= rhs;
  return *this;
}


Complex::Ptr::Expr Complex::Ptr::Expr::operator+(IntExpr b)  {
  return Expr(m_re + b, m_im + b);
}


// TODO: can this be combined with previous?
Complex::Ptr::Expr Complex::Ptr::operator+(IntExpr b)  {
  return Expr(m_re + b, m_im + b);
}


Complex::Ptr Complex::Ptr::mkArg() {
  V3DLib::Expr::Ptr re_e = Pointer::getUniformPtr();
  V3DLib::Expr::Ptr im_e = Pointer::getUniformPtr();

  Complex::Ptr x = ComplexExpr(re_e, im_e);
  return x;
}


bool Complex::Ptr::passParam(IntList &uniforms, Complex::Array *p) {
  uniforms.append(p->re().getAddress());
  uniforms.append(p->im().getAddress());
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Class Complex::Array
///////////////////////////////////////////////////////////////////////////////

Complex::Array::ref &Complex::Array::ref::operator=(complex const &rhs) {
  m_re_ref = rhs.re();
  m_im_ref = rhs.im();
  return *this;
}

Complex::Array::ref &Complex::Array::ref::operator=(ref const &rhs) {
  m_re_ref = rhs.m_re_ref;
  m_im_ref = rhs.m_im_ref;
  return *this;
}


std::string Complex::Array::ref::dump() const {
  auto tmp = complex(m_re_ref, m_im_ref);
  return tmp.dump();
}


bool Complex::Array::ref::operator==(complex const &rhs) const {
  return (m_re_ref == rhs.re() && m_im_ref == rhs.im());
}


bool Complex::Array::ref::operator==(ref const &rhs) const {
  return (m_re_ref == rhs.m_re_ref && m_im_ref == rhs.m_im_ref);
}


complex Complex::Array::ref::operator*(ref const &rhs) const {
  return complex(m_re_ref*rhs.m_re_ref - m_im_ref*rhs.m_im_ref, m_re_ref*rhs.m_im_ref + m_im_ref*rhs.m_re_ref);
}


complex Complex::Array::ref::to_complex() const {
  return complex(m_re_ref, m_im_ref);
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

Complex::Array::ref::ref(float const &re, float const &im) :
  m_re_const(re),
  m_im_const(im),
  m_re_ref(m_re_const),
  m_im_ref(m_im_const) {}


Complex::Array::ref Complex::Array::operator[] (int i) {
  return ref(m_re[i], m_im[i]);
}

Complex::Array::ref Complex::Array::operator[] (int i) const {
  return ref(m_re[i], m_im[i]);
}


///////////////////////////////////////////////////////////////////////////////
// Class Complex::2DArray
///////////////////////////////////////////////////////////////////////////////

Complex::Array2D::Array2D(int rows, int columns) : m_re(rows, columns),  m_im(rows, columns) {}

void Complex::Array2D::fill(complex val) {
  m_re.fill(val.re());
  m_im.fill(val.im());
}


int Complex::Array2D::rows() const {
  assert(m_re.rows() == m_im.rows());
  return m_re.rows();
}


int Complex::Array2D::columns() const {
  assert(m_re.columns() == m_im.columns());
  return m_re.columns();
}


std::string Complex::Array2D::dump() const {
  std::string ret;

  for (int r = 0; r < m_re.rows(); ++r) {
    ret << "(";
    for (int c = 0; c < m_re.columns(); ++c) {
      complex tmp(m_re[r][c], m_im[r][c]);
      ret << tmp.dump() << ", ";
    }
    ret << ")\n";
  }

  return ret;
}


void Complex::Array2D::make_unit_matrix() {
  assert(m_re.rows() == m_re.columns());  // square matrices only
  assert(m_re.rows() == m_im.rows());
  assert(m_re.columns() == m_im.columns());

  for (int r = 0; r < m_re.rows(); ++r) {
    for (int c = 0; c < m_re.columns(); ++c) {
      if (r ==c) {
        m_re[r][c] = 1.0f;
        m_im[r][c] = 0.0f;
      } else {
        m_re[r][c] = 0.0f;
        m_im[r][c] = 0.0f;
      }
    }
  }
}


void gather(Complex::Ptr const &addr) {
  gatherBaseExpr(addr.re());
  gatherBaseExpr(addr.im());
}


void receive(Complex &dst) {
  receive(dst.re());
  receive(dst.im());
}


}  // namespace V3DLib
