#include "Matrix.h"
#include "Support/debug.h"

namespace {
	int N = 1;  // Dimension of square matrix in blocks of 16 values.
}

////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////

float random_float() {
	return  (1.0f*(((float) (rand() % 200)) - 100.0f))/100.0f;  // Intention: values between -1 and 1
}


////////////////////////////////////////////////////////////////////////////////
// Kernel code definitions for Matric
////////////////////////////////////////////////////////////////////////////////

namespace kernels {

using namespace V3DLib;

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
 * Set value of src to vector element 'n' of dst
 *
 * All other values in dst are untouched.
 *
 * This is a kernel helper function.
 *
 * @param n  index of vector element to set. Must be in range 0..15 inclusive
 */
void set_at(Float &dst, Int n, Float &src) {
  Where(index() == n)
    dst = src;
  End 
}


/**
 * Sum up all the vector elements of a register.
 *
 * All vector elements of register result will contain the same value.
 *
 * This is a kernel helper function.
 */
void rotate_sum(Float &input, Float &result) {
  result = input;

  Float tmp = input;              comment("rotate_sum, loop unrolled");
  for (int i = 0; i < 15; i++) {  // loop unroll
    tmp = rotate(tmp, 1);
    result += tmp;
    //result = result + tmp;
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
    elements[i] = *input;  input += 16;
  }
}


void DotVector::save(Ptr<Float> output) {
  for (int i = 0; i < (int) elements.size(); ++i) {
    *output = elements[i];  output += 16;
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
    tmp = tmp + elements[i]*(*rhs);  rhs += 16;
  }

  rotate_sum(tmp, result);
}


////////////////////////////////////////////////////////////////////////////////
// End Class DotVector 
////////////////////////////////////////////////////////////////////////////////

void set_matrix_dim(int val) {
	N = val;
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
 * - unroll the internal loop (TODO)
 * - Use all QPU's (TODO)
 * - All QPU's iterate over b together -> increase cache hits
 */
void matrix_mult(Ptr<Float> dst, Ptr<Float> a, Ptr<Float> b) {
  int const DIM = 16*N;  // N is global static

  DotVector vec(N);
  Float result;

  For (Int a_index = 0,  a_index< DIM, a_index++)
    Ptr<Float> b_in = b + 0;  // Wonky '+ 0' to ensure pointer value is COPIED, not referenced.
    vec.load(a + 0);          // And again, and below again
                             // TODO fix this very NOT intuitive 'feature'. Bitten me >1 times.

    For (Int b_index = 0, b_index < DIM, b_index++)
      Float tmp;
      vec.dot_product(b_in + 0, tmp);

      set_at(result, b_index & 0xf, tmp);  // intention: b_index % 16

      If ((b_index & 0xf) == 15)
        *dst = result;
        dst += 16;
      End

      b_in += DIM;
    End  // IDIOT }  - never forget

    a += DIM;
  End
}


/**
 * Decorator for the matrix multiplication kernel.
 *
 * This passes in a value for the compilation, while leaving the prototype as is.
 *
 * NOTE: This function is not thread-safe. It sets a global static.
 *       Since currently multiple threads are neither used nor supported, 
 *       this is not an issue. 
 *
 * @param dimension  dimension of matrixes used in multiplication,
 *                   must be a multiple of 16
 */
FuncType *matrix_mult_decorator(int dimension) {
	assert(dimension > 0);
	assertq(dimension % 16 == 0, "dimension must be a multiple of 16");

	N = dimension >> 4;
	return matrix_mult;
}

}  // namespace kernels
