#ifndef _V3DLIB_KERNELS_MATRIX_H_
#define _V3DLIB_KERNELS_MATRIX_H_
#include "V3DLib.h"

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
  void save(Float::Ptr dst);
  void dot_product(Float::Ptr rhs, Float &result);
  void dft_dot_product(Int const &k, Complex &result);
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
  DEFAULT,      // No prefetch
  DO_PREFETCH,
  NO_READWRITE
};

void square_matrix_mult_scalar(int N, float *dst, float *a, float *b);
void matrix_mult(Float::Ptr dst, Float::Ptr a, Float::Ptr b);

using FuncType = decltype(matrix_mult);
FuncType *matrix_mult_decorator(int dimension, MatrixReadMethod read_method = DO_PREFETCH);

FuncType *matrix_mult_decorator(
  Float::Array2D &a,
  Float::Array2D &b,
  Float::Array2D &result,
  MatrixReadMethod read_method = DO_PREFETCH);


///////////////////////////////////////////////////////////////////////////////
// Complex arrays
///////////////////////////////////////////////////////////////////////////////

class ComplexDotVector {
public:
  ComplexDotVector(int size) : re(size), im(size) {}

  size_t size() const;

  void load(Complex::Ptr const &rhs);
  void load(Float::Ptr const &rhs);

  void save(Complex::Ptr output) {
    re.save(output.re());
    im.save(output.im());
  }

  void dot_product(Complex::Ptr rhs, Complex &result);
  void dft_dot_product(Int const &k, Complex &result);

private:
  DotVector re;
  DotVector im;
};


void complex_matrix_mult(Complex::Ptr dst, Complex::Ptr a, Complex::Ptr b);
using ComplexFuncType = decltype(complex_matrix_mult);

ComplexFuncType *complex_matrix_mult_decorator(
  Complex::Array2D &a,
  Complex::Array2D &b,
  Complex::Array2D &result,
  MatrixReadMethod read_method = DO_PREFETCH
);


///////////////////////////////////////////////////////////////////////////////
// DFT
///////////////////////////////////////////////////////////////////////////////

using DftFuncType = void (*)(Complex::Ptr dst, Complex::Ptr a);

DftFuncType dft_inline_decorator(
  Complex::Array2D &a,
  Complex::Array2D &result,
  MatrixReadMethod read_method = DO_PREFETCH
);


using DftFuncType2 = void (*)(Complex::Ptr dst, Float::Ptr a);

DftFuncType2 dft_inline_decorator(
  Float::Array &a,
  Complex::Array2D &result,
  MatrixReadMethod read_method = DO_PREFETCH
);


}  // namespace kernels


namespace V3DLib {

/**
 * Do block matrix multiplication
 *
 * Currently, the matrices are each split into 2 blocks.
 * This serves as a proof of concept; in due time it, is possible
 * to split them into any number of block matricesi, thereby allowing
 * arbitrary dimensions for the matrices (multiples of 16, always).
 *
 * mult() is here to be able to compare results with block_mult().
 * In due time, it will be removed.
 */
class Matrix {
 using KernelType = V3DLib::Kernel<Float::Ptr, Float::Ptr, Float::Ptr>;
 using BlockKernelType = V3DLib::Kernel<Float::Ptr, Float::Ptr, Float::Ptr, Int>;

public:
  Matrix(Float::Array2D &a, Float::Array2D &b);

  Float::Array2D &result() { return m_result; }

  void mult();
  KernelType &full_kernel() { return *k; }
  void full_compile()  { init_full(); }
  bool full_has_errors() const { return k->has_errors(); }

  void block_mult();
  BlockKernelType &block_kernel() { return *k_block; }
  void block_compile()  { init_block(); }
  bool block_has_errors() const { return k_block->has_errors(); }


private:
  Float::Array2D &m_a;
  Float::Array2D &m_b;
  Float::Array2D m_result;

  std::unique_ptr<KernelType> k;
  std::unique_ptr<BlockKernelType> k_block;

  void init_full();
  void init_block();
};

}  // namespace V3DLib

#endif  // _V3DLIB_KERNELS_MATRIX_H_
