#ifndef _V3DLIB_SOURCE_COMPLEX_H_
#define _V3DLIB_SOURCE_COMPLEX_H_
#include "Common/SharedArray.h"
//#include "Common/Seq.h"
#include "Ptr.h"
#include "Float.h"

namespace V3DLib {

struct ComplexExpr {
  Expr* expr;   // Abstract syntax tree

  // Constructors
  ComplexExpr();
  //Complex(float x);
};


class Complex {
public:
  enum {
    size = 2  // Size of instance in 32-bit values
  };

  class Ptr : public Pointer {
  public:
    Ptr(PtrExpr<Complex> rhs) : Pointer(rhs) {}

    static Ptr mkArg();
  };

  Complex() {}
  Complex(const Complex &rhs);
  Complex(PtrExpr<Float> input);

  Complex operator*(Complex rhs);
  Complex operator*=(Complex rhs);
  void operator=(Complex const &rhs);

  Float Re;
  Float Im;
};


/*
// error: expected initializer before ‘<’ token

template <> inline bool passParam< Complex::Ptr, SharedArray<Complex>* >
  (Seq<int32_t>* uniforms, SharedArray<Complex>* p)
{
  uniforms->append(p->getAddress());
  return true;
}
*/

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_COMPLEX_H_
