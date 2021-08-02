#include "DotVector.h"
#include "Support/basics.h"
#include "vc4/DMA/Operations.h"

namespace kernels {

DotVector::DotVector(int size) {
  assertq(size >= 1, "There must be at least one element for DotVector", true);
  elements.resize(size);  
}


void DotVector::load(Float::Ptr input) {
  int label = prefetch_label();

  for (int i = 0; i < (int) elements.size(); ++i) {
    prefetch(elements[i], input, label); // on v3d, TMU is used always
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
    prefetch(tmp2, rhs, label);  // on v3d, TMU is used always
    tmp += elements[i]*tmp2;
  }

  rotate_sum(tmp, result);
}


/**
 * Multiply current instance with the DFT elements of column `col`.
 *
 * The DFT matrix elements are calculated inline.
 * Note that low-precision sin/cos is used for vc4.
 *
 * @param row  index of row in dft array to process
 *
 * **TODO** use same header as ComplexDotVector::dft_dot_product().
 */
void DotVector::dft_dot_product(Int const &row, Complex &result, int num_elements, Int const &offset) {
  Complex tmp(0, 0);               comment("DotVector::dft_dot_product()");

  for (int i = 0; i < (int) size(); ++i) {
    Int col = (i*16 + index() + offset);  // Index of column to process
    Float param = -1.0f*toFloat(row*col)/toFloat(num_elements);
    Complex tmp1(elements[i]*cos(param), elements[i]*sin(param));

    tmp += tmp1;
  }

  rotate_sum(tmp.re(), result.re());
  rotate_sum(tmp.im(), result.im());
}


/**
 * on v3d, TMU is used always for writes.
 * on vc4, DMA is used always.
 */
void pre_write(Float::Ptr &dst, Float &src, bool add_result) {
  if (add_result) {
    int pre_label = prefetch_label();
    Float::Ptr dst_read = dst;

    Float tmp = 0;
    prefetch(tmp, dst_read, pre_label);
    *dst = tmp + src;
    dst.inc();
  } else {
    *dst = src;
    dst.inc();
  }
}


/**
 * Write first j values of src vector to dst
 */
void pre_write(Float::Ptr &dst, Float &src, bool add_result, Int const &j) {
  if (Platform::compiling_for_vc4()) {
    Float tmp = 0;

    if (add_result) {
      comment("vc4 float pre_write with add");
      int pre_label = prefetch_label();

      Float::Ptr dst_read = dst;

      prefetch(tmp, dst_read, pre_label);
      tmp += src;
    } else {
      comment("vc4 float pre_write no add");
      tmp = src;
    }

    vpmSetupWrite(HORIZ, me());
    vpmPut(tmp);

    dmaSetWriteStride((16 - j)*4);
    dmaSetupWrite(HORIZ, 1, 4*me(), j);
    dmaStartWrite(dst);
    dmaWaitWrite();   

    dst.inc();
  } else {
    Float::Ptr local_dst = dst;

    Where (index() >= j)
      local_dst = devnull();
    End

    pre_write(local_dst, src, add_result);
  }
}

}  // namespace kernels
