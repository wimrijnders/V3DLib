#include "Matrix.h"
#include <functional>
#include "Support/basics.h"


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

int N = 1;                                // Dimension of square matrix in blocks of 16 values.
MatrixReadMethod read_method = DEFAULT;

}  // anon namespace


////////////////////////////////////////////////////////////////////////////////
// Kernel Helper Functions
////////////////////////////////////////////////////////////////////////////////

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
 *   loop_unroll(DIM, 8, [&] (Int b_index) {
 *     // Same code as in loop above
 *   });
 */
void loop_unroll(int size, int unroll, std::function<void(Int)> f) {
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


void pre_read(Float &dst, Ptr<Float> &src, int prefetch_label) {
  // on v3d, TMU is used always

  switch (read_method) {
    case DEFAULT:
      // on vc4, this will use DMA
      dst = *src;
      src.inc();
      break;
    case USE_TMU:
      // on vc4, this will use TMU
      gather(src);
      receive(dst);
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


void pre_write(Ptr<Float> &dst, Float &src) {
  // on v3d, TMU is used always

  switch (read_method) {
    case DEFAULT:
    case USE_TMU:
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


////////////////////////////////////////////////////////////////////////////////
// Class DotVector 
////////////////////////////////////////////////////////////////////////////////

DotVector::DotVector(int size) {
  assertq(size >= 1, "There must be at least one element for DotVector");
  elements.resize(size);  
}


void DotVector::load(Ptr<Float> input) {
  for (int i = 0; i < (int) elements.size(); ++i) {
    pre_read(elements[i], input, 1);
  }
}


void DotVector::save(Ptr<Float> output) {
  for (int i = 0; i < (int) elements.size(); ++i) {
    pre_write(output, elements[i]);
  }
}


/**
 * Calculate the dot product of current instance and `rhs`.
 *
 * All vector elements of the result will contain the same value.
 */
void DotVector::dot_product(Ptr<Float> rhs, Float &result) {
  Float tmp = 0;  comment("DotVector::dot_product()");

  for (int i = 0; i < (int) elements.size(); ++i) {
    Float tmp2;
    pre_read(tmp2, rhs, 2);
    tmp += elements[i]*tmp2;
  }

  rotate_sum(tmp, result);
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
void matrix_mult_scalar(int N, float *c, float *a, float *b) {
  for (int x = 0; x < N; x++) {
    for (int y = 0; y < N; y++) {
      float result = 0;

      for (int i = 0; i < N; i++) {
        result += a[i + y*N] * b [x + i*N];
      }

      c[x + y*N] = result;
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
 * - Use prefetching on the TMU (TODO)
 * - unroll the internal loop (does not help, not implemented here)
 * - Use all QPU's (TODO)
 * - All QPU's iterate over b together -> increase cache hits
 * - Maybe utilize wait slots in branches (TODO)
 */
void matrix_mult(Ptr<Float> in_dst, Ptr<Float> a, Ptr<Float> b) {
  int const DIM = 16*N;  // N is global static
  Int STEP = DIM*numQPUs();

  a -= me()*16;
  b -= me()*16;
  in_dst -= me()*16;

  a += me()*DIM;

  DotVector vec(N);
  Float result;

  For (Int a_index = 0,  a_index < DIM, /* a_index++ */ a_index += numQPUs())
    Ptr<Float> dst = in_dst + (a_index + me())*DIM;
    Ptr<Float> b_in = b + 0;  // Wonky '+ 0' to ensure pointer value is COPIED, not referenced.
    vec.load(a + 0);          // And again, and below again
                             // TODO fix this very NOT intuitive 'feature'. Bitten me >1 times.

    For (Int b_index = 0, b_index < DIM, b_index++)
      Float tmp;
      vec.dot_product(b_in + 0, tmp);

      set_at(result, b_index & 0xf, tmp);  // intention: b_index % 16

      If ((b_index & 0xf) == 15)
        pre_write(dst, result);
      End

      b_in += DIM;
    End  // IDIOT }  - never forget

    //a += DIM;
    a += STEP;
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
 * @param dimension       dimension of matrixes used in multiplication,
 *                        must be a multiple of 16
 * @param in_do_readwrite if true, read/write to/from main memory (what you
 *                        normally expect). Otherwise, do the kernel operations only.
 *
 * @return  pointer to the actual kernel function
 */
FuncType *matrix_mult_decorator(int dimension, MatrixReadMethod in_read_method) {
  assert(dimension > 0);
  assertq(dimension % 16 == 0, "dimension must be a multiple of 16");

  N = dimension >> 4;
  read_method = in_read_method;

  return matrix_mult;
}

}  // namespace kernels
