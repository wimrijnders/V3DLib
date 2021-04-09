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


void compare_arrays(Complex::Array2D &a, Complex::Array2D &b, float precision) {
  REQUIRE(a.rows() == b.rows());
  REQUIRE(a.columns() == b.columns());

  if ( precision == -1.0f) {
    //precision = 1.0e-4f;   // for high precision sin/cos in kernels
    precision = 4.0e-1f;     // for low high precision sin/cos in kernels
  }

  for (int r = 0; r < a.rows(); ++r) {
    for (int c = 0; c < a.columns(); ++c) {
//      if (c == 0) {
        INFO("(r, c): ( " << r << ", " << c << ")");
        INFO(a[r][c].dump() << " == " << b[r][c].dump());

        // <= for dealing with precision 0.0f
//        REQUIRE(abs(a[r][c].magnitude() - b[r][c].magnitude()) <= precision);
        REQUIRE(abs(a[r][c].re() - b[r][c].re()) <= precision);
        REQUIRE(abs(a[r][c].im() - b[r][c].im()) <= precision);
//      }
    }
  }
}
