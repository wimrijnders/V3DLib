#include "matrix_support.h"
#include "../doctest.h"

using namespace V3DLib;


void compare_arrays(Float::Array2D &a, float const *b) {
  float precision = 0;

  // Values empirically determined - the bigger the matrices, the less precise
  if (Platform::has_vc4()) {
    precision = 1.0e-3f;
  } else {
    precision = 2.5e-4f;  // This value works for 640x640 matrices
  }

  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      INFO("r: " << r << ", c: " << c);
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
    precision = 4.0e-1f;     // for low  precision sin/cos in kernels
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
