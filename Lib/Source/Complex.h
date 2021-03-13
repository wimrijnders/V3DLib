#ifndef _V3DLIB_SOURCE_COMPLEX_H_
#define _V3DLIB_SOURCE_COMPLEX_H_
#include "Int.h"
#include "Float.h"

namespace V3DLib {

class Complex;


///////////////////////////////////////////////////////////////////////////////
// Class ComplexExpr
///////////////////////////////////////////////////////////////////////////////

class ComplexExpr {
public:
  ComplexExpr();
  ComplexExpr(Complex const &rhs);
  ComplexExpr(Expr::Ptr re, Expr::Ptr im) : re_e(re), im_e(im) {}

  Expr::Ptr re() { return re_e; }
  Expr::Ptr im() { return im_e; }

private:
  Expr::Ptr re_e = nullptr;   // Abstract syntax tree
  Expr::Ptr im_e = nullptr;   // TODO prob necessary to combine in single expr
};


///////////////////////////////////////////////////////////////////////////////
// Class complex
///////////////////////////////////////////////////////////////////////////////

/**
 * CPU-side complex definition
 */
class complex {
public:
  complex() = default;
  complex(float re, float im) : m_re(re), m_im(im) {}

  float re() const { return m_re; }
  float im() const { return m_im; }
  float magnitude() const;
  complex conjugate() const { return complex(m_re, -m_im); }
  std::string dump() const;

  bool operator==(complex const &rhs) const {
    return m_re == rhs.m_re && m_im == rhs.m_im;
  }

  complex operator*(complex const &rhs) const {
    return complex(m_re*rhs.m_re - m_im*rhs.m_im, m_re*rhs.m_im + m_im*rhs.m_re);
  }

  complex operator*(float rhs) const {
    complex tmp(rhs, 0);
    return complex(m_re*tmp.m_re - m_im*tmp.m_im, m_re*tmp.m_im + m_im*tmp.m_re);
  }


  complex &operator+=(complex const &rhs) {
    m_re += rhs.m_re;
    m_im += rhs.m_im;
    return *this;
  }


  complex &operator*=(float scalar) {
    m_re *= scalar;
    m_im *= scalar;
    return *this;
  }

private:
  float m_re = 0.0f;
  float m_im = 0.0f;
};


inline complex operator*(float lhs, complex rhs) {
  return rhs*lhs;
}


///////////////////////////////////////////////////////////////////////////////
// Class Complex
///////////////////////////////////////////////////////////////////////////////

/**
 * QPU-side complex definition
 */
class Complex {
public:
  enum {
    size = 2  // Size of instance in 32-bit values
  };


  /**
   * Encapsulates two disticnt shared float arrays for real and imaginary values
   */
  class Array {

    class ref {
    public:
      ref(float &re_ref, float &im_ref);

      ref &operator=(complex const &rhs);
      bool operator==(complex const &rhs) const;
      bool operator==(ref const &rhs) const;
      complex operator*(ref const &rhs) const;

      std::string dump() const;

    private:
      float &m_re_ref;
      float &m_im_ref;
    };

  public:
    Array(int size);

    size_t size() const {
      assert(m_re.size() == m_im.size());
      return m_re.size();
    }

    void fill(complex const &rhs);
    std::string dump() const;

    Float::Array &re() { return  m_re; }
    Float::Array &im() { return  m_im; }

    ref operator[] (int i);

  private:
    Float::Array m_re;
    Float::Array m_im;
  };


  class Array2D {
    struct Row {
      Row(Array2D &parent, int row, int row_size) :
        m_re(&parent.re(), row, row_size),
        m_im(&parent.im(), row, row_size)
        {}

      ~Row() {
        if (m_proxy_col != -1) {
          m_re[m_proxy_col] = m_proxy.re();
          m_im[m_proxy_col] = m_proxy.im();
        }
      }

      complex operator[] (int col) const { return complex(m_re[col], m_im[col]); }

      complex &operator[] (int col) {
        m_proxy = complex(m_re[col], m_im[col]);
        m_proxy_col = col;
        return m_proxy;
      }

    private:
      Float::Array2D::Row m_re;
      Float::Array2D::Row m_im;

      int   m_proxy_col = -1;
      complex m_proxy;
    };

  public:
    Array2D() = default;
    Array2D(int rows, int columns);
    Array2D(int dimension) : Array2D(dimension, dimension) {}

    Float::Array2D &re() { return  m_re; }
    Float::Array2D &im() { return  m_im; }

    void fill(complex val);
    int rows() const;
    int columns() const;

    void alloc(uint32_t rows, uint32_t columns) {
      m_re.alloc(rows, columns);
      m_im.alloc(rows, columns);
    }

    bool allocated() const { return m_re.allocated() && m_im.allocated(); }

    Row operator[] (int row) { return Row(*this, row, columns()); }

    void make_unit_matrix();
    std::string dump() const;

  private:
    Float::Array2D m_re;
    Float::Array2D m_im;
  };


  class Ptr {
  public:
    class Deref {
    public:
      Deref(V3DLib::Expr::Ptr re, V3DLib::Expr::Ptr im);

      Deref &operator=(Complex const &rhs);

      V3DLib::Deref<Float> m_re;
      V3DLib::Deref<Float> m_im;
    };

    class Expr {
    public:
      Expr(Float::Ptr re, Float::Ptr im) : m_re(re.expr()), m_im(im.expr()) {}
      Expr(Complex::Ptr e) : m_re(e.re().expr()), m_im(e.im().expr()) {}

      Ptr::Expr operator+(IntExpr b);

      V3DLib::PtrExpr<Float> m_re;
      V3DLib::PtrExpr<Float> m_im;
    };

    Ptr() = default;
    Ptr(ComplexExpr rhs);
    Ptr(Ptr::Expr rhs);

    Deref operator*();
    Ptr::Expr operator+(IntExpr b);
    Ptr &operator+=(IntExpr rhs);
    void inc() { m_re.inc(); m_im.inc(); }

    Float::Ptr &re() { return  m_re; }
    Float::Ptr &im() { return  m_im; }
    Float::Ptr const &re() const  { return  m_re; }
    Float::Ptr const &im() const  { return  m_im; }

    static Ptr mkArg();
    static bool passParam(IntList &uniforms, Complex::Array *p);

  private:
    Float::Ptr m_re;
    Float::Ptr m_im;
  };


  Complex() {}
  Complex(FloatExpr const &e_re, Float const &e_im);
  Complex(Complex const &rhs);
  Complex(ComplexExpr input);
  Complex(Ptr::Deref d);

  Float &re() { return m_re; }
  Float const &re() const { return m_re; }
  Float &im() { return m_im; }
  Float const &im() const { return m_im; }
  void re(FloatExpr const &e) { m_re = e; }
  void im(FloatExpr const &e) { m_im = e; }

  Float mag_square() const;

  Complex operator+(Complex rhs) const;
  Complex &operator+=(Complex rhs);
  Complex operator*(Complex rhs) const;
  Complex &operator*=(Complex rhs);
  void operator=(Complex const &rhs);

  void set_at(Int n, Complex const &src);

private:
  Float m_re;
  Float m_im;

  Complex &self();
};

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_COMPLEX_H_
