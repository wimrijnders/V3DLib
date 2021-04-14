#include "Matrix.h"
#include <functional>
#include "Support/basics.h"
#include "Source/Functions.h"


////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////

/**
 * Return a random float value between -1 and 1
 */ 
float random_float() {
  return  (1.0f*(((float) (rand() % 200)) - 100.0f))/100.0f;
}


namespace kernels {

using namespace V3DLib;

namespace {

struct matrix_settings {
  int rows;                                   // Rows of the result array
  int inner;                                  // Inner dimension of the multiplication
  int columns;                                // Columns of the result array
  MatrixReadMethod read_method = DO_PREFETCH;

  /**
   * The rows size of the result array needs to be a multiple of the number of QPUs running.
   *
   * This is a consequence of the for-loop in matrix_mult, which might be specified better.
   */
  int rows_result() const {
    return adjust_dimension(rows, 12);
  }


  /**
   * The column size of the result array needs to be a multiple of 16, i.e. vector size.
   */
  int cols_result() const {
    return adjust_dimension(columns, 16);
  }

private:

  int adjust_dimension(int val, int multiple) const {
    assert(val > 0);
    if (val % multiple != 0) {
      val  = multiple*(val/multiple + 1);
    }

    return val;
  }

} settings;


////////////////////////////////////////////////////////////////////////////////
// Kernel Helper Functions
////////////////////////////////////////////////////////////////////////////////

#if 0
/**
 * Works, but does not improve the performance of matrix in any way.
 * The reason for this is that the dotvector product is already unrolled.
 *
 * Will still be useful in other contexts.
 *
 * ## Usage
 *
 * Given a loop:
 * 
 *   For (Int b_index = 0, b_index < DIM, b_index++)
 *     // Code possibly using b_index
 *   End
 *
 * Replace with following:
 *
 *   loop_unroll(DIM, 8, [&] (Int const &n) {  // n is loop index (watch out for conflict with `index()`)
 *     // Same code as in loop above
 *   });
 */
void loop_unroll(int size, int unroll, std::function<void(Int const &)> f) {
  assert(size > 0);
  assert(unroll > 0);
  assert(size >= unroll);
  assertq(size % unroll == 0, "loop_unroll(): size must be a multiple of unroll");

  std::string cmt("Loop unroll ");
  cmt << unroll << " for size " << size;

  Int i = 0;  comment(cmt);
  For (, i < size, i += unroll)
    for (int j = 0; j < unroll; ++j) {
      f(i + j);
      cmt = "End loop unroll ";
      comment(cmt << j << "/" << unroll);
    }
  End
}
#endif  // 0


void pre_read(Float &dst, Float::Ptr &src, int prefetch_label) {
  // on v3d, TMU is used always

  switch (settings.read_method) {
    case DEFAULT:
      // on vc4, either TMU (default) or DMA (option)
      dst = *src;
      src.inc();
      break;
    case DO_PREFETCH:
      prefetch(dst, src, prefetch_label);
      break;
    case NO_READWRITE:
      dst = 0.0f;
      src.inc();
      break;
    default:
      assert(false);
  }
}


void pre_write(Float::Ptr &dst, Float &src) {
  // on v3d, TMU is used always

  switch (settings.read_method) {
    case DEFAULT:
    case DO_PREFETCH:
      // on vc4 this uses DMA
      // on v3d this uses TMU
      *dst = src;
      dst.inc();
      break;
    case NO_READWRITE:
      dst.inc();
      break;
    default:
      assert(false);
  }
}


void check_allocate_result_array(Complex::Array2D &result) {
  if(!result.allocated()) {
    // Result array requires column size which is a multiple of 16
    // Ensure enough padding for result so that size is multiple of 16
    // It may become too big but never mind
    result.alloc(settings.rows, settings.cols_result());
  } else {
    if (result.rows() != settings.rows) {
      std::string msg = "check_allocate_result_array(): result array "
                        "should have the same number of rows as matrix a ";
      msg << "(" << settings.rows << ")";
      assertq(msg);
    }

    if (result.columns() != settings.cols_result()) {
      std::string msg = "check_allocate_result_array(): result array should have a columns size of ";
      msg << settings.cols_result();
      assertq(msg);
    }
  }
}

}  // anon namespace


////////////////////////////////////////////////////////////////////////////////
// Class DotVector 
////////////////////////////////////////////////////////////////////////////////

DotVector::DotVector(int size) {
  assertq(size >= 1, "There must be at least one element for DotVector");
  elements.resize(size);  
}


void DotVector::load(Float::Ptr input) {
  for (int i = 0; i < (int) elements.size(); ++i) {
    pre_read(elements[i], input, 1);
  }
}


void DotVector::save(Float::Ptr output) {
  for (int i = 0; i < (int) elements.size(); ++i) {
    pre_write(output, elements[i]);
  }
}


/**
 * Calculate the dot product of current instance and `rhs`.
 *
 * All vector elements of the result will contain the same value.
 */
void DotVector::dot_product(Float::Ptr rhs, Float &result) {
  Float tmp = 0;  comment("DotVector::dot_product()");

  for (int i = 0; i < (int) elements.size(); ++i) {
    Float tmp2;
    pre_read(tmp2, rhs, 2);
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


///////////////////////////////////////////////////////////////////////////////n
// Kernels
////////////////////////////////////////////////////////////////////////////////

/**
 * CPU version of matrix multiplication, naive implementation
 *
 * Matrixes are assumed to be square.
 *
 * @param N  dimension of square matrices
 * @param c  Pointer to result array
 */
void square_matrix_mult_scalar(int N, float *dst, float *a, float *b) {
  for (int x = 0; x < N; x++) {
    for (int y = 0; y < N; y++) {
      float result = 0;

      for (int i = 0; i < N; i++) {
        result += a[i + y*N] * b [x + i*N];
      }

      dst[x + y*N] = result;
    }
  }
}


/**
 * Multiply two square matrixes
 *
 * Does a matrix multiplication of `a` and `b` and puts the result in `dst`.
 *
 * Input matrix `b` needs to be in transposed form before usage.
 * Template parameters N is dimension of square matrix in blocks of 16 values.
 *
 * ----------------------------------------------------------------------------
 * Optimizations
 * =============
 *
 * - Load one entire row of a into the QPU for fetching one single time
 * - Use prefetching on the TMU
 * - unroll the internal loop (tried it but does not help, not added)
 * - Use all QPU's
 * - All QPU's iterate over b together -> increase cache hits
 */
void matrix_mult(Float::Ptr dst, Float::Ptr a, Float::Ptr b) {
  assert(settings.inner > 0 && (settings.inner % 16 == 0));
  int const DIM = settings.inner;
  Int STEP = DIM*numQPUs();

  a += me()*DIM;

  DotVector vec(settings.inner/16);
  Float result = 0;  // NOTE explicit init required (TODO enforce)

  For (Int a_index = 0,  a_index < settings.rows, a_index += numQPUs())
    Float::Ptr dst_local = dst + (a_index + me())*settings.cols_result();
    Float::Ptr b_local = b;

    vec.load(a);

    Int b_index;

    For (b_index = 0, b_index < settings.columns, b_index++)
      Float tmp;
      vec.dot_product(b_local, tmp);

      set_at(result, b_index & 0xf, tmp);  // intention: b_index % 16

      If ((b_index & 0xf) == 15)
        pre_write(dst_local, result);
      End

      b_local += DIM;
    End  // IDIOT }  - never forget

    If ((b_index & 0xf) != 0)
      pre_write(dst_local, result);
    End

    // TODO make similar changes to other related kernels
    a += STEP; //DIM*numQPUs();  // Go to next row for current QPU
  End
}


///////////////////////////////////////////////////////////////////////////////n
// Decorator Function
////////////////////////////////////////////////////////////////////////////////

/**
 * Decorator for the matrix multiplication kernel.
 *
 * This passes in a value for the compilation, while leaving the prototype as is.
 *
 * **NOTE:** This function is not thread-safe, it sets global statics.
 *           Since currently multiple threads are neither used nor supported, 
 *           this is not an issue. 
 *
 * @param rows            number of rows in first matrix
 * @param inner           inner dimension of matrixes used in multiplication,
 *                        must be a multiple of 16
 * @param columns         number of columns in second matrix
 * @param in_do_readwrite if true, read/write to/from main memory (what you
 *                        normally expect). Otherwise, do the kernel operations only.
 *
 * @return  pointer to the actual kernel function
 */
FuncType *matrix_mult_decorator(int rows, int inner, int columns, MatrixReadMethod read_method) {
  assert(rows > 0);
  assert(columns > 0);
  assertq(inner % 16 == 0, "Inner dimension must be a multiple of 16");

  settings.rows        = rows;
  settings.inner       = inner;
  settings.columns     = columns;
  settings.read_method = read_method;

  return matrix_mult;
}


FuncType *matrix_mult_decorator(int dimension, MatrixReadMethod read_method) {
  return matrix_mult_decorator(dimension, dimension, dimension, read_method);
}


/**
 * Override with extra safety checks of matrix dimensions
 *
 * The result array should not have been allocated beforehand, done here.
 */
FuncType *matrix_mult_decorator(
  Float::Array2D &a,
  Float::Array2D &b,
  Float::Array2D &result,
  MatrixReadMethod read_method
) {
  assert(a.allocated());
  assert(b.allocated());
  assertq(!result.allocated(), "matrix_mult_decorator(): result array should not be allocated beforehand.");

  auto ret = matrix_mult_decorator(a.rows(), a.columns(), b.rows(), read_method);

  // Result array requires column size which is a multiple of 16
  // Ensure enough padding for result so that size is multiple of 16
  // It may become too big but never mind
  result.alloc(settings.rows_result(), settings.cols_result());

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Complex arrays
///////////////////////////////////////////////////////////////////////////////

size_t ComplexDotVector::size() const {
  assert(re.size() == im.size());
  return re.size();
}


void ComplexDotVector::load(Complex::Ptr const &rhs) {
  Float::Ptr rhs_re = rhs.re();  // Need to init ptr's here so that they are initialized before prefetch
  Float::Ptr rhs_im = rhs.im();

  for (int i = 0; i < (int) size(); ++i) {
    pre_read(re[i], rhs_re, 1);
    pre_read(im[i], rhs_im, 1);
  }
}


void ComplexDotVector::load(Float::Ptr const &rhs) {
  Float::Ptr rhs_re = rhs;  // Need to init ptr's here so that they are initialized before prefetch

  for (int i = 0; i < (int) size(); ++i) {
    pre_read(re[i], rhs_re, 1);
    im[i] = 0;
  }
}


void pre_write(Complex::Ptr &dst, Complex &src) {
  pre_write(dst.re(), src.re());
  pre_write(dst.im(), src.im());
}


void ComplexDotVector::dot_product(Complex::Ptr rhs, Complex &result) {
  Complex tmp(0, 0);               comment("ComplexDotVector::dot_product()");
  Float::Ptr rhs_re = rhs.re();
  Float::Ptr rhs_im = rhs.im();

  for (int i = 0; i < (int) size(); ++i) {
    Complex tmp1(re[i], im[i]);
    Float re2;
    Float im2;
    pre_read(re2, rhs_re, 2);
    pre_read(im2, rhs_im, 2);
    tmp += tmp1*Complex(re2, im2);
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


/**
 * Intentionally made to parallel `matrix_mult`, with the hope of combining
 * the code (template?).
 */
void complex_matrix_mult(Complex::Ptr dst, Complex::Ptr a, Complex::Ptr b) {
  assert(settings.inner > 0 && (settings.inner % 16 == 0));
  int const DIM = settings.inner;
  Int STEP = DIM*numQPUs();

  a += me()*DIM;

  ComplexDotVector vec(settings.inner/16);
  Complex result(0,0);  // NOTE explicit init required (TODO enforce)

  For (Int a_index = 0,  a_index < settings.rows, a_index += numQPUs())
    Complex::Ptr dst_local = dst + (a_index + me())*settings.cols_result();
    Complex::Ptr b_local = b;

    vec.load(a);

    Int b_index;

    For (b_index = 0, b_index < settings.columns, b_index++)
      Complex tmp;
      vec.dot_product(b_local, tmp);

      result.set_at(b_index & 0xf, tmp);  // intention: b_index % 16

      If ((b_index & 0xf) == 15)
        pre_write(dst_local, result);
      End

      b_local += DIM;
    End  // IDIOT }  - never forget

    If ((b_index & 0xf) != 0)
      pre_write(dst_local, result);
    End

    a += STEP;
  End
}


/**
 * Version of matrix mult, which allows for `a` to be an array with < 16 columns
 * (even 1), and not have a multiple of 16 as number of columns.
 *
 * As a benefit, this needs no columns alignment to 16 for result array.
 *
 * Needs to have the same prototype as `complex_matrix_mult()`.
 */
void complex_matrix_mult_1(Complex::Ptr dst, Complex::Ptr a, Complex::Ptr b) {
  assert(settings.inner > 0 && (settings.inner % 16 == 0));
  assert(settings.columns > 0 && (settings.columns % 16 == 0));

  int const DIM = settings.inner;

  ComplexDotVector vec(settings.inner/16);
  Complex result(0,0);  // NOTE explicit init required (TODO enforce)

  For (Int a_index = 0,  a_index < settings.rows, a_index += 1)
    vec.load(a);

    // b_index: column index of block of 16 columns to process by 1 QPU
    For (Int b_index = 16*me(), b_index < settings.columns, b_index += 16*numQPUs())
      Complex::Ptr b_local   = b + b_index*settings.inner;
      Complex::Ptr dst_local = dst + a_index*settings.cols_result() + b_index;
  
      Complex tmp;
      For (Int j = 0,  j < 16, j += 1)
        vec.dot_product(b_local, tmp);

        result.set_at(j & 0xf, tmp);
        b_local += settings.inner;
      End

      pre_write(dst_local, result);
    End

    a+= DIM;
  End
}


/**
 * Remember, b is transposed!
 */
ComplexFuncType *complex_matrix_mult_decorator(
  Complex::Array2D &a,
  Complex::Array2D &b,
  Complex::Array2D &result,
  MatrixReadMethod read_method
) {
  assert(a.allocated());
  assert(b.allocated());

  matrix_mult_decorator(a.rows(), a.columns(), b.rows(), read_method);
  check_allocate_result_array(result);

  if (a.rows() < 16 || a.rows() %16 != 0) {
    //printf("using complex_matrix_mult_1\n");
    return complex_matrix_mult_1;
  } else {
    //printf("using complex_matrix_mult\n");
    return complex_matrix_mult;
  }
}


///////////////////////////////////////////////////////////////////////////////
// DFT
///////////////////////////////////////////////////////////////////////////////



/**
 * Defined as a template so that complex input is possible.
 * This is useful if the reverse DFT is ever needed.
 *
 * Tried moving local vars out of the loops to avoid 'register allocation failed', didn't work 
 */
template<typename T, typename DotVecType>
void dft_inline_kernel(Complex::Ptr dst, T a) {
  assert(settings.inner > 0 && (settings.inner % 16 == 0));
  assert(settings.columns > 0 && (settings.columns % 16 == 0));

  int const DIM = settings.inner;

  DotVecType vec(settings.inner/16);

  Complex result(0,0);  // init required! Otherwise, var not added here in target lang
                        // This also applies to other local variables
                        // It's sort of a bug, but I'll live with it for now
                        // TODO examine in due time

  For (Int a_index = 0,  a_index < settings.rows, a_index += 1)
    vec.load(a);

    // b_index: column index of block of 16 columns to process by 1 QPU
    Int b_index = 0;
    For (b_index = 16*me(), b_index < settings.columns, b_index += 16*numQPUs())
      Int offset = (a_index*settings.cols_result() + b_index);  // Calculating offset first is slightly more efficient
      Complex::Ptr dst_local = dst + offset;
  
      Int j;
      For (j = 0,  j < 16, j += 1)
        Complex tmp(0,0);
        vec.dft_dot_product(b_index + j, tmp);
        result.set_at(j & 0xf, tmp);
      End

      pre_write(dst_local, result);
    End

    a+= DIM;
  End
}


DftFuncType dft_inline_decorator(Complex::Array2D &a, Complex::Array2D &result, MatrixReadMethod read_method) {
  assert(a.allocated());

  matrix_mult_decorator(a.rows(), a.columns(), a.columns(), read_method);
  check_allocate_result_array(result);

  return dft_inline_kernel<Complex::Ptr, ComplexDotVector>;
}


DftFuncType2 dft_inline_decorator(Float::Array &a, Complex::Array2D &result, MatrixReadMethod read_method) {
  assert(a.allocated());

  matrix_mult_decorator(1, a.size(), a.size(), read_method);
  check_allocate_result_array(result);

  return dft_inline_kernel<Float::Ptr, DotVector>;
}


}  // namespace kernels
