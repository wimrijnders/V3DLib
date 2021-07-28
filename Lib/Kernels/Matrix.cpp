#include "Matrix.h"
#include <functional>
#include "Support/basics.h"
#include "Source/Functions.h"

namespace kernels {

////////////////////////////////////////////////////////////////////////////////
// struct matrix_settings
////////////////////////////////////////////////////////////////////////////////

/**
 * NOTE: Strictly speaking, this is a reset.
 */
void matrix_settings::set(int in_rows, int in_inner, int in_columns) {
  assert(in_rows > 0);
  assert(in_columns > 0);
  assertq(in_inner % 16 == 0, "Inner dimension must be a multiple of 16");

  rows          = in_rows;
  inner         = in_inner;
  columns       = in_columns;
  add_result    = false;       // override after this call to explicitly set

  m_num_blocks  = -1;
  block_rowsize = -1;
}


void matrix_settings::set_blockrowsize(int in_block_rowsize) {
  assertq(inner > 0 && inner % 16 == 0, "Inner dimension must be a multiple of 16");
  assertq(inner % in_block_rowsize == 0, "Expecting block row size to be a multiple of inner");

  block_rowsize = in_block_rowsize;
}


/**
 * The number of consecutive values within a matrix row to handle
 * in the matrix multiplication
 */
int matrix_settings::width() const {
  if (m_num_blocks == -1 ) {
    return inner;
  }

  assert(block_rowsize != -1);
  return block_rowsize;
}


/**
 * The column size of the result array needs to be a multiple of 16, i.e. vector size.
 */
int matrix_settings::cols_result() const { return adjust_dimension(columns, 16); }


int matrix_settings::num_blocks() const {
  assertq(m_num_blocks == 1 || m_num_blocks == 2, "Num blocks can only be 1 or 2", true);
  return m_num_blocks;
}


void matrix_settings::num_blocks(int val) {
  assert(val == 1 || val == 2);

  int new_block_size = inner/val;
  assertq(new_block_size % 16 == 0, "New block size must be a multiple of 16");
  m_num_blocks = val;
  set_blockrowsize(new_block_size);
}


std::string matrix_settings::dump() const {
  std::string msg;

  msg << "settings "
      << "rows: " << rows << ", columns: " << columns
      << ", width: " << width() << ", inner: " << inner
      << ", num blocks: " << m_num_blocks;

  return msg;
}


int matrix_settings::adjust_dimension(int val, int multiple) const {
  assert(val > 0);
  if (val % multiple != 0) {
    val  = multiple*(val/multiple + 1);
  }

  return val;
}


using namespace V3DLib;

namespace {

matrix_settings settings;


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

}  // anon namespace


matrix_settings &get_matrix_settings() { return settings; }


/**
 * CPU version of matrix multiplication, naive implementation
 *
 * Matrixes are assumed to be square.
 *
 * @param N  dimension of square matrices
 * @param c  Pointer to result array
 */
void matrix_mult_scalar(int N, float *dst, float *a, float *b) {
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


void create_block_kernel(Int const &in_offset, std::function<void (Int const &offset)> f) {
  auto &settings = get_matrix_settings();

  if (settings.multi_block) {
    // Use a separate kernel for every offset
    f(in_offset);
  } else {
    // Use a single block for the offsets

    // Offset param ignored here

    // First call doesn't need to get the result values for addition; they are zero anyway
    settings.add_result = false;
    f(0);

    if (settings.num_blocks() == 2) {
      settings.add_result = true;
      Int offset = settings.width();
      f(offset);
    }
  }
}

}  // namespace kernels
