#include "Matrix.h"
#include <functional>
#include "Support/basics.h"
#include "Source/Functions.h"

namespace kernels {

////////////////////////////////////////////////////////////////////////////////
// struct matrix_settings
////////////////////////////////////////////////////////////////////////////////

void matrix_settings::set(int in_rows, int in_inner, int in_columns, int in_block_rowsize) {
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
  return block_rowsize;
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
  if (Platform::compiling_for_vc4()) {
    f(in_offset);
  } else {
    // Offset param ignored here
    auto &settings = get_matrix_settings();

    // First call doesn't need to get the result values for addition; they are zero anyway
    settings.add_result = false;
    f(0);

    assert(settings.num_blocks == 1 || settings.num_blocks == 2);
    if (settings.num_blocks == 2) {
      settings.add_result = true;
      Int offset = settings.block_rowsize;
      f(offset);
    }
  }
}

}  // namespace kernels
