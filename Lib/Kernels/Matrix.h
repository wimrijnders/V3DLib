#ifndef _V3DLIB_KERNELS_MATRIX_H_
#define _V3DLIB_KERNELS_MATRIX_H_
#include <type_traits>
#include "V3DLib.h"
#include "Support/basics.h"
#include "Support/Helpers.h"
#include "ComplexDotVector.h"

////////////////////////////////////////////////////////////////////////////////
// Kernel code definitions for Matrix
////////////////////////////////////////////////////////////////////////////////

namespace kernels {

using namespace V3DLib;

struct matrix_settings {
  int rows;                                   // Num rows of the result array
  int inner;                                  // Inner dimension of the multiplication
                                              // inner == columns of a == rows of b (which is transposed)
  int columns;                                // Num columns of the result array
  bool add_result = false;

  void set(int in_rows, int in_inner, int in_columns);

  int rows_result() const { return rows; }        //< Return the number of rows in the result array
  int width() const;
  int cols_result() const;
  int stride() const { return rows; }             //< Number of cells till next row
  int num_blocks() const;
  void num_blocks(int val);

private:
  int m_num_blocks  = -1;
  int block_rowsize = -1;                         // Row size for the (block array) multiplication

  int adjust_dimension(int val, int multiple) const;
  void set_blockrowsize(int in_block_rowsize);
};


matrix_settings &get_matrix_settings();


/**
 * Pre: settings initialized
 *
 * If result has already been initialized, does a check on dimensions.
 * Otherwise, properly initialize results.
 */
template<typename Array2D>
void init_result_array(Array2D &result) {
  auto &settings = get_matrix_settings();

  if (!result.allocated()) {
    // Result array requires column size which is a multiple of 16
    // Ensure enough padding for result so that size is multiple of 16
    // It may become too big but never mind
    result.alloc(settings.rows, settings.cols_result());
  } else {
    if (result.rows() != settings.rows) {
      std::string msg = "init_result_array(): result array "
                        "should have the same number of rows as matrix a ";
      msg << "(" << settings.rows << ")";
      assertq(msg);
    }

    if (result.columns() != settings.cols_result()) {
      std::string msg = "init_result_array(): result array should have a columns size of ";
      msg << settings.cols_result();
      assertq(msg);
    }
  }
}


/**
 * Multiply two matrixes
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
template<typename Ptr>
void matrix_mult(Ptr dst, Ptr a, Ptr b) {
  //debug("matrix_mult kernel");
  auto &settings = get_matrix_settings();

  assert(settings.inner > 0 && (settings.inner % 16 == 0));

  using T          = typename std::conditional<std::is_same<Ptr, Float::Ptr>::value, Float, Complex>::type;
  using DotVecType = typename std::conditional<std::is_same<Ptr, Float::Ptr>::value, DotVector, ComplexDotVector>::type;

  DotVecType vec(settings.width()/16);
  T result = 0;  // NOTE explicit init required (TODO enforce)

  For (Int a_index = 0,  a_index < settings.rows, a_index += 1)
    vec.load(a);

    // b_index: column index of block of 16 columns to process by 1 QPU
    For (Int b_index = 16*me(), b_index < settings.columns, b_index += 16*numQPUs())
      Ptr b_local   = b + b_index*settings.inner;
      Ptr dst_local = dst + a_index*settings.cols_result() + b_index;
  
      T tmp;
      For (Int j = 0,  j < 16, j += 1)
        vec.dot_product(b_local, tmp);

        result.set_at(j & 0xf, tmp);
        b_local += settings.inner;
      End

      pre_write(dst_local, result, settings.add_result);
    End

    a += settings.inner;  // jump to next row
  End
}


////////////////////////////////////////////////////////////////////////////////
// API functions
////////////////////////////////////////////////////////////////////////////////

void matrix_mult_scalar(int N, float *dst, float *a, float *b);


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
inline auto matrix_mult_decorator(int rows, int inner, int columns) -> decltype(*matrix_mult<Float::Ptr>) {
  auto &settings = get_matrix_settings();

  settings.set(rows, inner, columns);
  return matrix_mult<Float::Ptr>;
}


using FuncType = void (Float::Ptr dst, Float::Ptr a, Float::Ptr b);
inline FuncType *matrix_mult_decorator(int dimension) {
  return matrix_mult_decorator(dimension, dimension, dimension);
}


/**
 * Decorator for the matrix multiplication kernel.
 *
 * Has extra safety checks of matrix dimensions.
 * Remember, b is transposed!
 */
template<
  typename Array2D,
  typename Ptr = typename std::conditional<std::is_same<Array2D, Float::Array2D>::value, Float::Ptr, Complex::Ptr>::type
>
inline auto matrix_mult_decorator(Array2D &a, Array2D &b, Array2D &result) -> decltype(*matrix_mult<Ptr>) {
  assert(a.allocated());
  assert(b.allocated());

  auto &settings = get_matrix_settings();

  settings.set(a.rows(), a.columns(), b.rows());
  init_result_array(result);
  return matrix_mult<Ptr>;
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
template<typename Ptr>
void dft_kernel_intern(Complex::Ptr dst, Ptr a, Int const &offset) {
  auto &settings = get_matrix_settings();

  assert(settings.inner > 0 && (settings.inner % 16 == 0));
  assert(settings.columns > 0 && (settings.columns % 16 == 0));

  using DotVecType = typename std::conditional<std::is_same<Ptr, Float::Ptr>::value, DotVector, ComplexDotVector>::type;

  DotVecType vec(settings.width()/16);

  Complex result(0,0);  // init required! Otherwise, var not added here in target lang
                        // This also applies to other local variables
                        // It's sort of a bug, but I'll live with it for now
                        // TODO examine in due time

  For (Int a_index = 0,  a_index < settings.rows, a_index += 1)
    vec.load(a);

    // b_index: column index of block of 16 columns to process by 1 QPU
    For (Int b_index = 16*me(), b_index < settings.columns, b_index += 16*numQPUs())
      Int dst_offset = (a_index*settings.cols_result() + b_index);  // Calculating offset first is slightly more efficient
      Complex::Ptr dst_local = dst + dst_offset;

      Complex tmp(0,0);
      For (Int j = 0,  j < 16, j += 1)
        vec.dft_dot_product(b_index + j, tmp, offset);
        result.set_at(j & 0xf, tmp);
      End

      pre_write(dst_local, result, settings.add_result);
    End

    a += settings.inner;  // jump to next row
  End
}


template<typename Ptr>
void dft_kernel(Complex::Ptr dst, Ptr a) {
  dft_kernel_intern(dst, a, 0);
}


template<typename Ptr>
void dft_kernel_block(Complex::Ptr in_dst, Ptr in_a, Int in_offset) {
  create_block_kernel(in_offset, [&] (Int const &offset) {
     dft_kernel_intern<Ptr>(in_dst, in_a + offset, offset);
  });
}


template<
  typename Array,
  typename Ptr = typename std::conditional<std::is_same<Array, Complex::Array2D>::value, Complex::Ptr, Float::Ptr>::type
>
auto dft_decorator(Array &a, Complex::Array2D &result) -> decltype(*dft_kernel<Ptr>) {
  assert(a.allocated());

  int rows, columns;

  if constexpr (std::is_same_v<Array, Complex::Array2D>) {
    rows    = a.rows();
    columns = a.columns();
  } else {
    rows    = 1;
    columns = a.size();
  }

  matrix_mult_decorator(rows, columns, columns);
  init_result_array(result);

  return dft_kernel<Ptr>;
}


void create_block_kernel(Int const &in_offset, std::function<void (Int const &offset)> f);


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
template<typename Ptr>
void matrix_mult_block(Ptr in_dst, Ptr in_a, Ptr in_b, Int in_offset) {
  create_block_kernel(in_offset, [&] (Int const &offset) {
     matrix_mult<Ptr>(in_dst, in_a + offset, in_b + offset);
  });
}

}  // namespace kernels


namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class Matrix
///////////////////////////////////////////////////////////////////////////////

/**
 * Base class for  block matrix support
 *
 * Currently, the matrices are each split into  at most 2 blocks.
 *
 * This serves as a proof of concept; in due time it, is possible
 * to split them into any number of block matrices, thereby allowing
 * arbitrary dimensions for the matrices (multiples of 16, always).
 */
template<
  typename Array,
  typename Ptr,
  typename BlockKernelType,
  typename ResultArray = Array
>
class BlockMatrix {
public:
  enum {
    DEFAULT_NUM_BLOCKS  =  -1,  // Let instance figure out itself whether to use full or block mult

    // Following values are empirically determined (i.e. by trying out)
    // There is actually some point in lowering the max value for vc4, because block mult is more efficient
    MAX_FULL_BLOCKS_VC4 = 800,  // Highest dimension where full mult can be used for vc4
    MAX_FULL_BLOCKS_V3D = 800,  // Highest dimension where full mult can be used for v3d
  };

  using BlockKernelPtr = std::unique_ptr<BlockKernelType>;


  ResultArray &result() { return m_result; }
  BlockKernelType &kernel() { return *m_k; }
  void compile()  { init_block(); }
  bool has_errors() const { return m_k_first_vc4->has_errors() || m_k->has_errors(); }


  void setNumQPUs(int val) {
    if (m_k_first_vc4.get() != nullptr) m_k_first_vc4->setNumQPUs(val);
    if (m_k.get() != nullptr) m_k->setNumQPUs(val);
  }


  void num_blocks(int val) {
    assert(DEFAULT_NUM_BLOCKS == val || 0 < val);
    if (val > 0) {
      assertq(val == 1 || val == 2, "Number of block matrices can only be 1 or 2" );
      auto &settings = kernels::get_matrix_settings();

      if (settings.inner/val % 16 != 0) {
        using ::operator<<;  // C++ weirdness

        std::string msg;
        msg << "Inner dimension (" << settings.inner << ") "
            << "must be a multiple of 16*<number of blocks> (" << val << ") "
            << "for block multiplication to work";
        assertq(false, msg);
      } 
    }

    m_num_blocks = val;
  }


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
  void call() {
    init_block();
    assertq(!has_errors(), "Can not run Matrix::mult(), there are errors");
    assert(m_k.get() != nullptr);
    bool const do_call = true;  // Can be set to false when using interpret() or emu() instead of call() below

    m_result.fill(0.0f);        // Apparently necessary; 1-ones mult -> final element is + 1 for some reason

    if (Platform::has_vc4() && do_call) {
      //debug("vc4 mult_block");
      // This part required for vc4 hardware; see header of kernel matrix_mult_block().
      assert(m_k_first_vc4.get() != nullptr);

      // First call doesn't need to get the result values for addition; they are zero anyway
      load(m_k_first_vc4, 0);
      m_k_first_vc4->call();

      if (num_blocks() == 2) {
        debug("Calling second block");
        auto &settings = kernels::get_matrix_settings();
        int offset = settings.width();
        load(m_k, offset);
        m_k->call();
      }
    } else {
      // This part would also work for interpret() and emu()
      //debug("v3d mult_block");
      load(m_k, 0);
      m_k->call();
    }
  }


protected:
  virtual void init_block() = 0;


  /**
   * Prepare the block matrix multiplication
   */
  template<typename KernelType>  
  void init_block_kernels(KernelType kernel) {
    auto &settings = kernels::get_matrix_settings();

    if (m_k.get() != nullptr) {
      // Kernel already compiled. Don't recompile if nothing changed
      if (settings.num_blocks() == num_blocks()) {
        //debug("Unchanged block");
        return;
      }
      //debug("Recompiling block");
    }

    settings.num_blocks(num_blocks());
    kernels::init_result_array(m_result);

    if (num_blocks() ==2) {
      debug("Doing 2 blocks");
    }

    //if (Platform::has_vc4()) {  TODO
      settings.add_result = false;
      m_k_first_vc4.reset(new BlockKernelType(V3DLib::compile(kernel)));

      if (m_k_first_vc4->has_errors()) {
        warning("compile failed of first kernel");
        m_k.reset(nullptr);
        return;
      }
    //}

    settings.add_result = true;
    m_k.reset(new BlockKernelType(V3DLib::compile(kernel)));
  }

protected:
  virtual void load(BlockKernelPtr &k, int offset) = 0;

private:
  int m_num_blocks = DEFAULT_NUM_BLOCKS;
  ResultArray m_result;

  std::unique_ptr<BlockKernelType> m_k;
  std::unique_ptr<BlockKernelType> m_k_first_vc4;


  /**
   * Determine number of blocks to use
   */
  int num_blocks() const {
    assert(MAX_FULL_BLOCKS_VC4 == MAX_FULL_BLOCKS_V3D);  // Handle this when it happens

    if (m_num_blocks == DEFAULT_NUM_BLOCKS) {
      auto &settings = kernels::get_matrix_settings();
      assert(settings.inner > 0);
      return (settings.inner <= MAX_FULL_BLOCKS_VC4)? 1: 2;
    }

    return m_num_blocks;
  }
};


/**
 * Do block matrix multiplication
 */
template<
  typename Array2D,
  typename Ptr = typename std::conditional<std::is_same<Array2D, Float::Array2D>::value, Float::Ptr, Complex::Ptr>::type,
  typename BlockKernelType = V3DLib::Kernel<Ptr, Ptr, Ptr, Int>,
  typename Parent = BlockMatrix<Array2D, Ptr, BlockKernelType>
>
class Matrix : public Parent {
public:
  Matrix(Array2D &a, Array2D &b) : m_a(a), m_b(b) {
    auto &settings = kernels::get_matrix_settings();
    settings.set(m_a.rows(), m_a.columns(), m_b.rows());
  }

  void load(std::unique_ptr<BlockKernelType> &k, int offset) override {
    k->load(&Parent::result(), &m_a, &m_b, offset);
  } 

  void init_block() override {
    Parent::init_block_kernels(kernels::matrix_mult_block<Ptr>);
  }

private:
  Array2D &m_a;
  Array2D &m_b;
};


/**
 * DFT with block matrix support
 *
 * Output is always a complex 2D array.
 */
template<
  typename Array,
  typename Ptr = typename std::conditional<std::is_same<Array, Float::Array>::value, Float::Ptr, Complex::Ptr>::type,
  typename BlockKernelType = V3DLib::Kernel<Complex::Ptr, Ptr, Int>,
  typename Parent = BlockMatrix<Array, Ptr, BlockKernelType, Complex::Array2D>
>
class DFT : public Parent {
public:
  DFT(Array &a) : m_a(a) {
    auto &settings = kernels::get_matrix_settings();
    settings.set(1, m_a.size(), m_a.size());
  }

  void load(std::unique_ptr<BlockKernelType> &k, int offset) override {
    k->load(&Parent::result(), &m_a, offset);
  } 

  void init_block() override {
    Parent::init_block_kernels(kernels::dft_kernel_block<Ptr>);
  }

private:
  Array &m_a;
};

}  // namespace V3DLib

#endif  // _V3DLIB_KERNELS_MATRIX_H_
