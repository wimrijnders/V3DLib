#ifndef _V3DLIB_KERNELS_COMPLEXDOTVECTOR_H_
#define _V3DLIB_KERNELS_COMPLEXDOTVECTOR_H_
#include "DotVector.h"

namespace kernels {

class ComplexDotVector {
public:
  ComplexDotVector(int size) : re(size), im(size) {}

  size_t size() const;

  void load(Complex::Ptr const &rhs);
  void load(Float::Ptr const &rhs);

  void save(Complex::Ptr output) {
    re.save(output.re());
    im.save(output.im());
  }

  void dot_product(Complex::Ptr rhs, Complex &result);
  void dft_dot_product(Int const &k, Complex &result);

private:
  DotVector re;
  DotVector im;
};


void pre_write(Complex::Ptr &dst, Complex &src, bool add_result);


}  // namespace kernels


#endif  // _V3DLIB_KERNELS_COMPLEXDOTVECTOR_H_
