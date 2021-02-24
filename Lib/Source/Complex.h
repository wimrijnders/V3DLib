#ifndef _V3DLIB_SOURCE_COMPLEX_H_
#define _V3DLIB_SOURCE_COMPLEX_H_
#include "Common/SharedArray.h"
#include "Int.h"
#include "Float.h"

namespace V3DLib {

class Complex;

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


/**
 * CPU-side complex definition
 */
class complex {
public:
  complex(float re, float im) : m_re(re), m_im(im) {}

  float re() const { return m_re; }
  float im() const { return m_im; }
  std::string dump() const;

  bool operator==(complex const &rhs) const {
    return m_re == rhs.m_re && m_im == rhs.m_im;
  }

  complex operator*(complex const &rhs) const {
    return complex(m_re*rhs.m_re - m_im*rhs.m_im, m_re*rhs.m_im + m_im*rhs.m_re);
  }


  complex &operator+=(complex const &rhs) {
    m_re += rhs.m_re;
    m_im += rhs.m_im;
    return *this;
  }

private:
  float m_re;
  float m_im;
};


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

    SharedArray<float> &re() { return  m_re; }
    SharedArray<float> &im() { return  m_im; }

    ref operator[] (int i);

  private:
    SharedArray<float> m_re;
    SharedArray<float> m_im;
  };


  class Array2D {
    struct Row {
      Row(Array2D &parent, int row, int row_size) :
        m_re(&parent.re(), row, row_size),
        m_im(&parent.im(), row, row_size)
        {}

      complex operator[] (int col) const { return complex(m_re[col], m_im[col]); }

    private:
      Shared2DArray<float>::Row m_re;
      Shared2DArray<float>::Row m_im;
    };

  public:
    Array2D() = default;
    Array2D(int rows, int columns);

    Shared2DArray<float> &re() { return  m_re; }
    Shared2DArray<float> &im() { return  m_im; }

    void fill(complex val);
    int rows() const;
    int columns() const;

    void alloc(uint32_t rows, uint32_t columns) {
      m_re.alloc(rows, columns);
      m_im.alloc(rows, columns);
    }

    bool allocated() const { return m_re.allocated() && m_im.allocated(); }

    Row operator[] (int row) { return Row(*this, row, columns()); }

  private:
    Shared2DArray<float> m_re;
    Shared2DArray<float> m_im;
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
      Expr(Complex::Ptr e) : m_re(e.re().expr()), m_im(e.im().expr()) {}

      V3DLib::PtrExpr<Float> m_re;
      V3DLib::PtrExpr<Float> m_im;
    };

    Ptr(ComplexExpr rhs);
    Ptr(Ptr::Expr rhs);

    Deref operator*();
    Ptr::Expr operator+(IntExpr b);
    Ptr &operator+=(IntExpr rhs);

    Float::Ptr &re() { return  m_re; }
    Float::Ptr &im() { return  m_im; }
    Float::Ptr const &re() const  { return  m_re; }
    Float::Ptr const &im() const  { return  m_im; }

    static Ptr mkArg();
    static bool passParam(Seq<int32_t> *uniforms, Complex::Array *p);

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
