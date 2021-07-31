#include "ComplexDotVector.h"
#include "vc4/DMA/Operations.h"

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


void pre_write(Complex::Ptr &dst, Complex &src, bool add_result, Int const &j) {
  if (Platform::compiling_for_vc4()) {
    Complex tmp = 0;

    if (add_result) {
      int pre_label = prefetch_label();
      Complex::Ptr dst_read = dst;
      pre_read(tmp, dst_read, pre_label);

      tmp += src;
    } else {
      tmp = src;
    }

    vpmSetupWrite(HORIZ, 2*me());
    vpmPut(tmp.re());
    vpmPut(tmp.im());

    dmaSetWriteStride((16 - j)*4);
    dmaSetupWrite(HORIZ, 1, 4*2*me(), j);
    dmaStartWrite(dst.re());
    dmaWaitWrite();   
    dmaSetupWrite(HORIZ, 1, 4*(2*me() + 1), j);
    dmaStartWrite(dst.im());
    dmaWaitWrite();   

    dst.inc();
  } else {
    Complex::Ptr local_dst = dst;

    Where (index() >= j)
      local_dst.re() = devnull();  comment("pre_write complex set top bits dst to devnull");
      local_dst.im() = devnull();
    End

    pre_write(local_dst, src, add_result);
  }

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
/*
  debug("complex dot_product");
  {
    std::string msg;
    msg << "size: " << size();
    debug(msg);
  }
*/

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
 * Multiply current instance with the DFT elements of column `col`.
 *
 * The DFT matrix elements are calculated inline.
 * Note that low-precision sin/cos is used for vc4.
 *
 * @param row          index of row in dft array to process
 * @param result       out parameter, to which result of dot product will be stored
 * @param num_elements total number of elements to use in divisor for phase.
 *                     For full multiplication, this will be the same as the dot vector size;
 *                     for block multiplication, this will be a multiple of size, the multiple
 *                     being the number of blocks.
 * @param offset       offset of num elements to use in columns for block matrix multiplication
 */
void ComplexDotVector::dft_dot_product(Int const &row, Complex &result, int num_elements, Int const &offset) {
  Complex tmp(0, 0);               comment("ComplexDotVector::dft_dot_product()");

  for (int i = 0; i < (int) size(); ++i) {
    Int col = (i*16 + index() + offset);  // Index of column to process
    Float param = -1.0f*toFloat(row*col)/toFloat(num_elements);
    Complex tmp1(re[i], im[i]);
    Complex tmp2(cos(param), sin(param));

    tmp += tmp1*tmp2;
  }

  rotate_sum(tmp.re(), result.re());
  rotate_sum(tmp.im(), result.im());
}

}  // namespace kernels
