#include "matrix_support.h"
#include "../doctest.h"
#include "Support/Helpers.h"  // random_float()

using namespace V3DLib;


void compare_arrays(Float::Array2D &a, float const *b, float precision) {

  // Values empirically determined - the bigger the matrices, the less precise
  if (precision == -1.0f) {
    if (Platform::has_vc4()) {
      precision = 1.0e-3f;
    } else {
      precision = 2.5e-4f;  // This value works for 640x640 matrices
    }
  }

  float max_diff = -1;

  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      float diff = abs(a[r][c] - b[r*a.columns() + c]);
      if (max_diff == -1 || max_diff < diff) {
        max_diff = diff;
      }
    }
  }

  INFO("Max diff: " << max_diff << ", precision: " << precision);

  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      INFO("r: " << r << ", c: " << c);
      //INFO("Result: " << a.dump());
      INFO("result: " << a[r][c] << ", expected: " << b[r*a.columns() + c]);
      REQUIRE(abs(a[r][c] - b[r*a.columns() + c]) < precision);
    }
  }
}


/**
 * This is better than:
 *
 *    REQUIRE(m1.result() == m2.result());
 *
 *    ....which has been known to fail incorrectly.
 */
void compare_arrays(Float::Array2D &a, Float::Array2D &b, float precision) {
  REQUIRE(a.rows() == b.rows());
  REQUIRE(a.columns() == b.columns());

  if ( precision == -1.0f) {
    //precision = 1.0e-4f;   // for high precision sin/cos in kernels
    precision = 4.1e-1f;     // for low  precision sin/cos in vc4 kernels (yeah, it sucks)
  }

  for (int r = 0; r < a.rows(); ++r) {
    for (int c = 0; c < a.columns(); ++c) {
      INFO("(r, c): ( " << r << ", " << c << ")");
      INFO(a[r][c] << " == " << b[r][c]);

      // <= for dealing with precision 0.0f
      REQUIRE(abs(a[r][c] - b[r][c]) <= precision);
      REQUIRE(abs(a[r][c] - b[r][c]) <= precision);
    }
  }
}


void compare_arrays(Complex::Array2D &a, Complex::Array2D &b, float precision) {
  REQUIRE(a.rows() == b.rows());
  REQUIRE(a.columns() == b.columns());

  if ( precision == -1.0f) {
    //precision = 1.0e-4f;   // for high precision sin/cos in kernels
    precision = 4.0e-1f;     // for low  precision sin/cos in kernels
  }

  float max_diff_re = -1;
  float max_diff_im = -1;

  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      float diff_re = abs(a[r][c].re() - b[r][c].re());
      if (max_diff_re == -1 || max_diff_re < diff_re) {
        max_diff_re = diff_re;
      }

      float diff_im = abs(a[r][c].im() - b[r][c].im());
      if (max_diff_im == -1 || max_diff_im < diff_im) {
        max_diff_im = diff_im;
      }
    }
  }

  // Do an overall check
  if (max_diff_re <= precision && max_diff_im <= precision) {
    return; // All is well
  }

  INFO("Max diff re: "   << max_diff_re
    << ", max diff im: " << max_diff_im
    << ", precision: "   << precision);


  // Do a specific check to find the (r,c) coordinate where it goes wrong
  for (int r = 0; r < a.rows(); ++r) {
    for (int c = 0; c < a.columns(); ++c) {
      INFO("(r, c): ( " << r << ", " << c << ")");
      INFO(a[r][c].dump() << " == " << b[r][c].dump());

      // <= for dealing with precision 0.0f
//        REQUIRE(abs(a[r][c].magnitude() - b[r][c].magnitude()) <= precision);
      REQUIRE(abs(a[r][c].re() - b[r][c].re()) <= precision);
      REQUIRE(abs(a[r][c].im() - b[r][c].im()) <= precision);
    }
  }
}


void compare_arrays(std::vector<float> &a, float const *b) {
  float precision = 1e-4f;

  for (int r = 0; r < (int) a.size(); ++r) {
    REQUIRE(abs(a[r] - b[r]) < precision);
  }
}


void check_unitary(std::vector<float> &a, int dim) {
  for (int r = 0; r < dim; r++) {
    for (int c = 0; c < dim; c++) {
      INFO("rows: " << dim << ", (r,c): (" << r << ", " << c << ")");
      int offset = r*dim + c;
      if (r == c) {
        REQUIRE(a[offset] == 1.0f);
      } else {
        REQUIRE(a[offset] == 0.0f);
      }
    }
  }
}


void check_unitary(Float::Array2D &a) {
  REQUIRE(a.rows() == a.columns());

  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      INFO("rows: " << a.rows() << ", (r,c): (" << r << ", " << c << ")");
      if (r == c) {
        REQUIRE(a[r][c] == 1.0f);
      } else {
        REQUIRE(a[r][c] == 0.0f);
      }
    }
  }
}


void fill_random(float *arr, int size) {
  for (int n = 0; n < size; n++) {
    arr[n] = random_float();
  }
}


void fill_random(std::vector<float> &arr) {
  assert(!arr.empty());
  fill_random(arr.data(), (int) arr.size());
}


/**
 * Pre: dst properly initialized, matches with src
 */
void copy_array(Float::Array2D &dst, float const *src) {
  for (int r = 0; r < dst.rows(); r++) {
    for (int c = 0; c < dst.columns(); c++) {
      dst[r][c] = src[r*dst.columns() + c];
    }
  }
}


void copy_array(Float::Array2D &dst, std::vector<float> const &src) {
  assert(!src.empty());
  assert((int) src.size() == dst.rows()*dst.columns());
  copy_array(dst, src.data());
}


void copy_transposed(float *dst, float const *src, int rows, int columns) {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < columns; c++) {
      dst[c*rows + r] = src[r*columns + c];
    }
  }
}


void copy_transposed(std::vector<float> &dst, std::vector<float> const &src, int rows, int columns) {
  copy_transposed(dst.data(), src.data(), rows, columns);
}


void compare_array_scalar(Float::Array2D &arr, float scalar) {
  for (int r = 0; r < arr.rows(); ++r) {
    for (int c = 0; c < arr.columns(); ++c) {
      INFO("r: " << r << ", c: " << c);
      INFO("result: " << arr[r][c] << ", expected: " << scalar);
      REQUIRE(arr[r][c] == scalar);
    }
  }
}

