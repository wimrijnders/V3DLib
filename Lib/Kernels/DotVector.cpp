#include "DotVector.h"
#include "Support/basics.h"

namespace kernels {
namespace {

/**
 * on v3d, TMU is used always for writes.
 * on vc4, DMA is used always.
 */
void pre_write(Float::Ptr &dst, Float::Ptr &dst_read, Float &src, int pre_label, bool add_result) {
  if (add_result) {
    //Intention: *dst = *dst + src;

    Float tmp = 0;
    prefetch(tmp, dst_read, pre_label);
    *dst = tmp + src;
  } else {
    *dst = src;
    dst.inc();
  }
}

}  // anon namespace;

DotVector::DotVector(int size) {
  assertq(size >= 1, "There must be at least one element for DotVector");
  elements.resize(size);  
}


void DotVector::load(Float::Ptr input) {
  int label = prefetch_label();

  for (int i = 0; i < (int) elements.size(); ++i) {
    pre_read(elements[i], input, label);
  }
}


void DotVector::save(Float::Ptr dst) {
  for (int i = 0; i < (int) elements.size(); ++i) {
    *dst = elements[i];
    dst.inc();
  }
}


/**
 * Calculate the dot product of current instance and `rhs`.
 *
 * All vector elements of the result will contain the same value.
 */
void DotVector::dot_product(Float::Ptr rhs, Float &result) {
  int label = prefetch_label();
  Float tmp = 0;  comment("DotVector::dot_product()");

  for (int i = 0; i < (int) elements.size(); ++i) {
    Float tmp2;
    pre_read(tmp2, rhs, label);
    tmp += elements[i]*tmp2;
  }

  rotate_sum(tmp, result);
}


/**
 * Multiply current instance with the DFT elements of line `k`.
 *
 * The DFT matrix elements are calculated inline.
 * Note that low-precision sin/cos is used for vc4.
 */
void DotVector::dft_dot_product(Int const &k, Complex &result) {
  Complex tmp(0, 0);               comment("DotVector::dft_dot_product()");

  int num_elements = ((int) size())* 16;
  for (int i = 0; i < (int) size(); ++i) {
    Float param = -1.0f*toFloat(k*(i*16 + index()))/toFloat(num_elements);

    Complex tmp1(elements[i]*cos(param), elements[i]*sin(param));

    tmp += tmp1;
  }

  rotate_sum(tmp.re(), result.re());
  rotate_sum(tmp.im(), result.im());
}


int prefetch_label() {
  static int count = 0;

  count++;
  return count;
}


void pre_read(Float &dst, Float::Ptr &src, int prefetch_label) {
  // on v3d, TMU is used always
  prefetch(dst, src, prefetch_label);
}


void pre_write(Float::Ptr &dst, Float &src, bool add_result) {
  int pre_label = prefetch_label();
  Float::Ptr dst_read = dst;
  pre_write(dst, dst_read, src, pre_label, add_result);
}

}  // namespace kernels
