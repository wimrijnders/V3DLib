#ifndef _EXAMPLES_KERNELS_MATRIX_H_
#define _EXAMPLES_KERNELS_MATRIX_H_
#include <V3DLib.h>

////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////

float random_float();


////////////////////////////////////////////////////////////////////////////////
// Kernel code definitions for Matrix
////////////////////////////////////////////////////////////////////////////////

namespace kernels {

using namespace V3DLib;


////////////////////////////////////////////////////////////////////////////////
// Class DotVector
////////////////////////////////////////////////////////////////////////////////

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

  void load(Float::Ptr input);
  void save(Float::Ptr output);
  void dot_product(Float::Ptr rhs, Float &result);
  size_t size() const { return elements.size(); }
  Float &operator[] (int index) { return elements[index]; }
  Float const &operator[] (int index) const { return elements[index]; }

private:
  std::vector<Float> elements;
};


////////////////////////////////////////////////////////////////////////////////
// API functions
////////////////////////////////////////////////////////////////////////////////

enum MatrixReadMethod {
  DEFAULT,
  DO_PREFETCH,
  NO_READWRITE
};

void square_matrix_mult_scalar(int N, float *c, float *a, float *b);
void matrix_mult(Float::Ptr dst, Float::Ptr a, Float::Ptr b);

using FuncType = decltype(matrix_mult);

FuncType *matrix_mult_decorator(int dimension, MatrixReadMethod read_method = DEFAULT);


FuncType *matrix_mult_decorator(
  Shared2DArray<float> &a,
  Shared2DArray<float> &b,
  Shared2DArray<float> &result,
  MatrixReadMethod read_method = DEFAULT);


///////////////////////////////////////////////////////////////////////////////
// Complex arrays
///////////////////////////////////////////////////////////////////////////////

class ComplexDotVector {
public:
  ComplexDotVector(int size) : re(size), im(size) {}

  size_t size() const;

  void load(Complex::Ptr const &rhs);

  void save(Complex::Ptr output) {
    re.save(output.re());
    im.save(output.im());
  }

  void dot_product(Complex::Ptr rhs, Complex &result);

private:
  DotVector re;
  DotVector im;
};


void complex_matrix_mult(Complex::Ptr dst, Complex::Ptr a, Complex::Ptr b);

using ComplexFuncType = decltype(complex_matrix_mult);


ComplexFuncType *complex_matrix_mult_decorator(
  Shared2DArray<complex> &a,
  Shared2DArray<complex> &b,
  Shared2DArray<complex> &result,
  MatrixReadMethod read_method
);


}  // namespace kernels

#endif  // _EXAMPLES_KERNELS_MATRIX_H_
