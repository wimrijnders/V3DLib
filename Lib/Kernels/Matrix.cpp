#include "Matrix.h"
#include <functional>
#include "Support/basics.h"
#include "Source/Functions.h"


////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////


namespace kernels {

using namespace V3DLib;

namespace {

struct matrix_settings {
  int rows;                                   // Num rows of the result array
  int num_blocks = -1;
  int block_rowsize;                          // Row size for the (block array) multiplication
  int inner;                                  // Inner dimension of the multiplication
                                              // inner == columns of a == rows of b (which is transposed)
  int columns;                                // Num columns of the result array

  MatrixReadMethod read_method = DO_PREFETCH;
  bool add_result = false;

  void set(
    int in_rows,
    int in_inner,
    int in_columns,
    int in_block_rowsize = -1
  ) {
    assert(in_rows > 0);
    assert(in_columns > 0);
    assertq(in_inner % 16 == 0, "Inner dimension must be a multiple of 16");

    rows          = in_rows;
    inner         = in_inner;
    columns       = in_columns;
    add_result    = false;       // override after this call to explicitly set

    if (in_block_rowsize == -1) {
      block_rowsize = in_inner;
    } else {
      set_blockrowsize(in_block_rowsize);
    }
  }


  void set_blockrowsize(int in_block_rowsize) {
    assertq(inner > 0 && inner % 16 == 0, "Inner dimension must be a multiple of 16");
    assertq(inner % in_block_rowsize == 0, "Expecting block rows to be a multiple of inner");

    block_rowsize = in_block_rowsize;
  }


  /**
   * The rows size of the result array needs to be a multiple of the number of QPUs running.
   *
   * This is a consequence of the for-loop in matrix_mult, which might be specified better.
   *
   * TODO: Check if edit is acceptable for all #QPUs, make concrete if so.
   */
  int rows_result() const {
    //return adjust_dimension(rows, 12);
    return rows;
  }


  /**
   * The column size of the result array needs to be a multiple of 16, i.e. vector size.
   */
  int cols_result() const {
    return adjust_dimension(columns, 16);
  }


  /**
   * Number of cells till next row
   */
  int stride() {
    return rows;
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

int prefetch_label() {
  static int count = 0;

  count++;
  return count;
}


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


/**
 * on v3d, TMU is used always for writes.
 * on vc4, DMA is used always.
 */
void pre_write(Float::Ptr &dst, Float::Ptr &dst_read, Float &src, int pre_label) {

  switch (settings.read_method) {
    case DEFAULT:
      *dst = src;
      dst.inc();
    break;
    case DO_PREFETCH:

      if (settings.add_result) {
       //debug("Doing add prefetch");
       //Intention: *dst = *dst + src;

       Float tmp = 0;
       prefetch(tmp, dst_read, pre_label);
       *dst = tmp + src;
      } else {
       *dst = src;
      }
      dst.inc();
      break;
    case NO_READWRITE:
      dst.inc();
      break;
    default:
      assert(false);
  }
}


void pre_write(Float::Ptr &dst, Float &src) {
  int pre_label = prefetch_label();
  Float::Ptr dst_read = dst;
  pre_write(dst, dst_read, src, pre_label);
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


/**
 * Pre: settings initialized
 */
void init_result_array(Float::Array2D &result) {
  if (result.allocated()) {
    assertq(result.rows()    == settings.rows_result(), "Preallocated result array has incorrect number of rows");
    assertq(result.columns() == settings.cols_result(), "Preallocated result array has incorrect number of columns");
  } else {
    // Result array requires column size which is a multiple of 16
    // Ensure enough padding for result so that size is multiple of 16
    // It may become too big but never mind
    result.alloc(settings.rows_result(), settings.cols_result());
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
  int label = prefetch_label();

  for (int i = 0; i < (int) elements.size(); ++i) {
    pre_read(elements[i], input, label);
  }
}


void DotVector::save(Float::Ptr dst) {
  for (int i = 0; i < (int) elements.size(); ++i) {
    pre_write(dst, elements[i]);
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
  Int STEP = settings.stride()*numQPUs();

  a += me()*settings.stride();

  DotVector vec(settings.block_rowsize/16);  comment("DotVector init");
  Float result = 0;  // NOTE explicit init required (TODO enforce)

  For (Int a_index = me(), a_index < settings.rows, a_index += numQPUs())
    Float::Ptr dst_local = dst + a_index*settings.cols_result();
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

      b_local += settings.stride();
    End  // IDIOT }  - never forget

    // TODO check if following relevant
    If ((b_index & 0xf) != 0)
      pre_write(dst_local, result);
    End

    // TODO make similar changes to other related kernels
    a += STEP; //DIM*numQPUs();  // Go to next row for current QPU
  End
}


/**
 * The v3d part of the kernel does not work on vc4, even though the target lang code
 * is practically identical. It took a while to figure it out:
 *
 *  - vc4 reads using the TMU and writes using VPM/DMA.
 *  - There is a cache before the TMU part
 *  - The DMA writes directly to memory; this does not invalidate the cache (!)
 *  - On the second call to matrix_mult(), the cache data is used, which by then 
 *    is not the same as memory
 *
 * The v3d does not have this issue, since it reads as well as writes with TMU,
 * thus always using the cache.
 * It drove me crazy a whole day. I solved it by calling the kernel >1 times
 * with different offset parameters for vc4.
 */ 
void matrix_mult_block(Float::Ptr in_dst, Float::Ptr in_a, Float::Ptr in_b, Int in_offset) {
  if (Platform::compiling_for_vc4()) {
    matrix_mult(in_dst, in_a + in_offset, in_b + in_offset);
  } else {
    // Offset param ignored here
    using kernels::settings;

    // First call doesn't need to get the result values for addition; they are zero anyway
    settings.add_result = false;
    matrix_mult(in_dst, in_a, in_b);

    assert(settings.num_blocks == 1 || settings.num_blocks == 2);
    if (settings.num_blocks == 2) {
      settings.add_result = true;
      Int offset = settings.block_rowsize;
      matrix_mult(in_dst, in_a + offset, in_b + offset);
    }
  }
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
FuncType *matrix_mult_decorator(
  int rows,
  int inner,
  int columns,
  MatrixReadMethod read_method
) {
  settings.set(rows, inner, columns);
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

  auto ret = matrix_mult_decorator(a.rows(), a.columns(), b.rows(), read_method);

  init_result_array(result);

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
  int label = prefetch_label();
  Float::Ptr rhs_re = rhs.re();  // Need to init ptr's here so that they are initialized before prefetch
  Float::Ptr rhs_im = rhs.im();

  for (int i = 0; i < (int) size(); ++i) {
    pre_read(re[i], rhs_re, label);
    pre_read(im[i], rhs_im, label);
  }
}


void ComplexDotVector::load(Float::Ptr const &rhs) {
  int label = prefetch_label();
  Float::Ptr rhs_re = rhs;  // Need to init ptr's here so that they are initialized before prefetch

  for (int i = 0; i < (int) size(); ++i) {
    pre_read(re[i], rhs_re, label);
    im[i] = 0;
  }
}


void pre_write(Complex::Ptr &dst, Complex &src) {
  pre_write(dst.re(), src.re());
  pre_write(dst.im(), src.im());
}


void ComplexDotVector::dot_product(Complex::Ptr rhs, Complex &result) {
  int label = prefetch_label();
  Complex tmp(0, 0);               comment("ComplexDotVector::dot_product()");
  Float::Ptr rhs_re = rhs.re();
  Float::Ptr rhs_im = rhs.im();

  for (int i = 0; i < (int) size(); ++i) {
    Complex tmp1(re[i], im[i]);
    Float re2;
    Float im2;
    pre_read(re2, rhs_re, label);
    pre_read(im2, rhs_im, label);
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

  For (Int a_index = me(),  a_index < settings.rows, a_index += numQPUs())
    Complex::Ptr dst_local = dst + a_index*settings.cols_result();
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

namespace V3DLib {

Matrix::Matrix(Float::Array2D &a, Float::Array2D &b) : m_a(a), m_b(b) { }


/**
 * This multiplies the input matrices using block matrix calculation,
 * with the following block matrices:
 * 
 *                        | B1 |
 *    AxB = | A1 | A2 | x | -- | = | A1xB1 + B1xB2 ]
 *                        | B2 |
 *
 * ...where the split dimension is halved for A1/A2 and B1/B2.
 * 
 * Further splitting is possible, but this serves our purposes for now.
 */
void Matrix::mult() {
  init_block();
  assertq(!has_errors(), "Can not run Matrix::mult(), there are errors");
  assert(m_k.get() != nullptr);
  bool const do_call = true;  // Can be set to false when using interpret() or emu() instead of call() below

  m_result.fill(0.0f);        // Apparently necessary; 1-ones mult -> final element is + 1 for some reason

  if (Platform::has_vc4() && do_call) {
    // This part required for vc4 hardware; see header of kernel matrix_mult_block().
    assert(m_k_first_vc4.get() != nullptr);

    // First call doesn't need to get the result values for addition; they are zero anyway
    m_k_first_vc4->load(&m_result, &m_a, &m_b, 0);
    m_k_first_vc4->call();

    if (num_blocks() == 2) {
      //debug("Calling second block");
      int offset = kernels::settings.block_rowsize;
      m_k->load(&m_result, &m_a, &m_b, offset);
      m_k->call();
   }
  } else {
    // This part would also work for interpret() and emu()
    m_k->load(&m_result, &m_a, &m_b, 0);
    m_k->call();
  }
}


/**
 * Prepare the block matrix multiplication
 */
void Matrix::init_block() {
  using kernels::settings;


  if (m_k.get() != nullptr) {
    if (settings.num_blocks == num_blocks()) {
      return;
    }
    //debug("Recompiling block");
  }

  settings.set(m_a.rows(), m_a.columns(), m_b.rows());
  int new_block_size = m_a.columns()/num_blocks();
  settings.num_blocks = num_blocks();
  settings.set_blockrowsize(new_block_size);
  kernels::init_result_array(m_result);

  settings.add_result = false;
  m_k_first_vc4.reset(new BlockKernelType(V3DLib::compile(kernels::matrix_mult_block)));

  if (m_k_first_vc4->has_errors()) {
    warning("compile failed of first kernel");
    m_k.reset(nullptr);
    return;
  }

  settings.add_result = true;
  m_k.reset(new BlockKernelType(V3DLib::compile(kernels::matrix_mult_block)));
  //m_k->pretty(true, "block_mult_vc4.txt");
  //m_k->pretty(false, "block_mult_v3d.txt");
  //m_k->dump_compile_data(true, "block_mult_data_vc4.txt");
}


void Matrix::setNumQPUs(int val) {
  if (m_k_first_vc4.get() != nullptr) m_k_first_vc4->setNumQPUs(val);
  if (m_k.get() != nullptr) m_k->setNumQPUs(val);
}


void Matrix::num_blocks(int val) {
  assert(DEFAULT_NUM_BLOCKS == val || 0 < val);
  if (val > 0) {
    assertq(val == 1 || val == 2, "Number of block matrices can only be 1 or 2" );
  }

  m_num_blocks = val;
}


/**
 * Determine number of blocks to use
 */
int Matrix::num_blocks() const {
  assert(MAX_FULL_BLOCKS_VC4 == MAX_FULL_BLOCKS_V3D);  // Handle this when it happens

  if (m_num_blocks == DEFAULT_NUM_BLOCKS) {
    assert(m_a.columns() > 0);
    return (m_a.columns() <= MAX_FULL_BLOCKS_VC4)? 1: 2;
  }

  return m_num_blocks;
}

}  // namespace V3DLib

