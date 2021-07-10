#ifndef _V3DLIB_KERNELS_MATRIX_H_
#define _V3DLIB_KERNELS_MATRIX_H_
#include "V3DLib.h"
#include "Support/Helpers.h"

////////////////////////////////////////////////////////////////////////////////
// Kernel code definitions for Matrix
////////////////////////////////////////////////////////////////////////////////

namespace kernels {

using namespace V3DLib;


////////////////////////////////////////////////////////////////////////////////
// API functions
////////////////////////////////////////////////////////////////////////////////

void square_matrix_mult_scalar(int N, float *dst, float *a, float *b);

using FuncType = void (Float::Ptr dst, Float::Ptr a, Float::Ptr b);
FuncType *matrix_mult_decorator(int dimension);
FuncType *matrix_mult_decorator(Float::Array2D &a, Float::Array2D &b, Float::Array2D &result);


///////////////////////////////////////////////////////////////////////////////
// Complex arrays
///////////////////////////////////////////////////////////////////////////////

using ComplexFuncType = void (Complex::Ptr dst, Complex::Ptr a, Complex::Ptr b);
ComplexFuncType *complex_matrix_mult_decorator(Complex::Array2D &a, Complex::Array2D &b, Complex::Array2D &result);


///////////////////////////////////////////////////////////////////////////////
// DFT
///////////////////////////////////////////////////////////////////////////////

using DftFuncType = void (*)(Complex::Ptr dst, Complex::Ptr a);
DftFuncType dft_inline_decorator(Complex::Array2D &a, Complex::Array2D &result);


using DftFuncType2 = void (*)(Complex::Ptr dst, Float::Ptr a);
DftFuncType2 dft_inline_decorator(Float::Array &a, Complex::Array2D &result);

}  // namespace kernels


namespace V3DLib {

/**
 * Do block matrix multiplication
 *
 * Currently, the matrices are each split into 2 blocks.
 * This serves as a proof of concept; in due time it, is possible
 * to split them into any number of block matrices, thereby allowing
 * arbitrary dimensions for the matrices (multiples of 16, always).
 */
class Matrix {
 using KernelType = V3DLib::Kernel<Float::Ptr, Float::Ptr, Float::Ptr>;
 using BlockKernelType = V3DLib::Kernel<Float::Ptr, Float::Ptr, Float::Ptr, Int>;

public:

  enum {
    DEFAULT_NUM_BLOCKS  =  -1,  // Let instance figure out itself whether to use full or block mult

    // Following values are empirically determined (i.e. by trying out)
    // There is actually some point in lowering the max value for vc4, because block mult is more efficient
    MAX_FULL_BLOCKS_VC4 = 800,  // Highest dimension where full mult can be used for vc4
    MAX_FULL_BLOCKS_V3D = 800,  // Highest dimension where full mult can be used for v3d
  };

  Matrix(Float::Array2D &a, Float::Array2D &b);

  void setNumQPUs(int val);
  void num_blocks(int val);
  Float::Array2D &result() { return m_result; }

  void mult();
  BlockKernelType &kernel() { return *m_k; }
  void compile()  { init_block(); }
  bool has_errors() const { return m_k_first_vc4->has_errors() || m_k->has_errors(); }


private:
  int m_num_blocks      = DEFAULT_NUM_BLOCKS;
  Float::Array2D &m_a;
  Float::Array2D &m_b;
  Float::Array2D m_result;

  std::unique_ptr<BlockKernelType> m_k;
  std::unique_ptr<BlockKernelType> m_k_first_vc4;

  void init_block();

  int num_blocks() const;
};

}  // namespace V3DLib

#endif  // _V3DLIB_KERNELS_MATRIX_H_
