#ifndef _EXAMPLES_KERNELS_MATRIX_H_
#define _EXAMPLES_KERNELS_MATRIX_H_
#include <V3DLib.h>

////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////

float random_float();


////////////////////////////////////////////////////////////////////////////////
// Kernel code definitions for Matric
////////////////////////////////////////////////////////////////////////////////

namespace kernels {

using namespace V3DLib;


////////////////////////////////////////////////////////////////////////////////
// Internal functions - should not be called directly
////////////////////////////////////////////////////////////////////////////////

void matrix_mult_kernel(int const N, Ptr<Float> &dst, Ptr<Float> &a, Ptr<Float> &b);


////////////////////////////////////////////////////////////////////////////////
// Kernel code, made visible for unit tests
////////////////////////////////////////////////////////////////////////////////

void set_at(Float &dst, Int n, Float &src);
void rotate_sum(Float &input, Float &result);


/**
 * Kernel helper class for loading in a sequence of values into QPU registers
 *
 * A Number of registers in the register file are allocated for the sequence.
 * These registers are indexed to retain the order.
 * 16 consequent values are loaded into the vector of a register.
 *
 * The goal here is to have the entire sequence of values loaded into the QPU
 * register file, so that it can be reused.
 * This, of course, places an upper limit on the length of the sequence.
 */
class DotVector {
public:
  DotVector(int size);

  void load(Ptr<Float> input);
  void save(Ptr<Float> output);
	void dot_product(Ptr<Float> rhs, Float &result);

private:
  std::vector<Float> elements;
};


////////////////////////////////////////////////////////////////////////////////
// API functions
////////////////////////////////////////////////////////////////////////////////

void matrix_mult_scalar(int N, float *c, float *a, float *b);

void matrix_mult(Ptr<Float> dst, Ptr<Float> a, Ptr<Float> b);

using FuncType = decltype(matrix_mult);

FuncType *matrix_mult_decorator(int N);

}  // namespace kernels

#endif  // _EXAMPLES_KERNELS_MATRIX_H_
