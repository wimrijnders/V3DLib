#include "ComplexDotVector.h"

namespace kernels {
namespace {

void pre_read(Complex &dst, Complex::Ptr &src, int prefetch_label) {
  prefetch(dst.re(), src.re(), prefetch_label); // on v3d, TMU is used always
  prefetch(dst.im(), src.im(), prefetch_label);
}

}  // anon namespace


void pre_write(Complex::Ptr &dst, Complex &src, bool add_result) {
  pre_write(dst.re(), src.re(), add_result);
  pre_write(dst.im(), src.im(), add_result);
}


size_t ComplexDotVector::size() const {
  assert(re.size() == im.size());
  return re.size();
}


void ComplexDotVector::load(Complex::Ptr const &rhs) {
  int label = prefetch_label();
  Float::Ptr rhs_re = rhs.re();  // Need to init ptr's here so that they are initialized before prefetch
  Float::Ptr rhs_im = rhs.im();

  for (int i = 0; i < (int) size(); ++i) {
    prefetch(re[i], rhs_re, label); // on v3d, TMU is used always
    prefetch(im[i], rhs_im, label); // on v3d, TMU is used always
  }
}


void ComplexDotVector::load(Float::Ptr const &rhs) {
  int label = prefetch_label();
  Float::Ptr rhs_re = rhs;  // Need to init ptr's here so that they are initialized before prefetch

  for (int i = 0; i < (int) size(); ++i) {
    prefetch(re[i], rhs_re, label);
    im[i] = 0;
  }
}


void ComplexDotVector::dot_product(Complex::Ptr rhs, Complex &result) {
  //debug("complex dot_product");
  int label = prefetch_label();
  Complex tmp(0, 0);               comment("ComplexDotVector::dot_product()");
  Complex tmp2(0, 0);

  for (int i = 0; i < (int) size(); ++i) {
    pre_read(tmp2, rhs, label);
    tmp += Complex(re[i], im[i])*tmp2;
  }

  rotate_sum(tmp.re(), result.re());
  rotate_sum(tmp.im(), result.im());
}


/**
 * Multiply current instance with the DFT elements of line `k`.
 *
 * The DFT matrix elements are calculated inline.
 * Note that low-precision sin/cos is used for vc4.
 */
void ComplexDotVector::dft_dot_product(Int const &k, Complex &result) {
  Complex tmp(0, 0);               comment("ComplexDotVector::dft_dot_product()");

  int num_elements = ((int) size())* 16;
  for (int i = 0; i < (int) size(); ++i) {
    Float param = -1.0f*toFloat(k*(i*16 + index()))/toFloat(num_elements);
    Complex tmp1(re[i], im[i]);
    Complex tmp2(cos(param), sin(param));

    tmp += tmp1*tmp2;
  }

  rotate_sum(tmp.re(), result.re());
  rotate_sum(tmp.im(), result.im());
}

}  // namespace kernels
