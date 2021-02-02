#include "catch.hpp"
#include <math.h>
#include <cmath>  // frexp()
#include "../Examples/Kernels/Rot3D.h"

using namespace kernels;


// ============================================================================
// Support routines
// ============================================================================

/**
 * Convenience method to initialize arrays.
 */
template<typename Arr>
void initArrays(Arr &x, Arr &y, int size, float mult = 1.0f) {
  for (int i = 0; i < size; i++) {
    x[i] = mult*((float) i);
    y[i] = mult*((float) i);
  }
}


template<typename Array1, typename Array2>
void compareResults(
  Array1 &x1,
  Array1 &y1,
  Array2 &x2,
  Array2 &y2,
  int size,
  const char *label,
  bool compare_exact = true) {
  for (int i = 0; i < size; i++) {
    INFO("Comparing " << label << " for index " << i);
    if (compare_exact) {
      INFO("y2[" << i << "]: " << y2[i]);
      REQUIRE(x1[i] == x2[i]);

/*
      int exp;  // To avoid 0.0f == -0.0f (rhs is extremely small)
      frexp(y2[i], &exp);

      float rhs = y2[i];
      if (exp != 0 && exp <= -100) {  // -126 <= exponent values <= 127
        printf("rhs: %f, exp: %d \n", rhs, exp);
        rhs = 0.0f;
      }
      REQUIRE(y1[i] == rhs);
*/
      REQUIRE(y1[i] == y2[i]);
    } else {
      REQUIRE(x1[i] == Approx(x2[i]).epsilon(0.001));
      REQUIRE(y1[i] == Approx(y2[i]).epsilon(0.001));
    }
  }
}


// ============================================================================
// The actual tests
// ============================================================================

TEST_CASE("Test working of Rot3D example", "[rot3d]") {
  // Number of vertices and angle of rotation
  const int N = 19200; // 192000
  const float THETA = (float) 3.14159;


  /**
   * Check that the Rot3D kernels return precisely what we expect.
   *
   * The scalar version of the algorithm may return slightly different
   * values than the actual QPU's, but they should be close. This is
   * because the hardware QPU's round downward in floating point
   * calculations
   *
   * If the code is compiled for emulator only (QPU_MODE=0), this
   * test will fail.
   */
  SECTION("All kernel versions should return the same") {
    //
    // Run the scalar version
    //

    // Allocate and initialise
    float* x_scalar = new float [N];
    float* y_scalar = new float [N];
    initArrays(x_scalar, y_scalar, N);

    rot3D(N, cosf(THETA), sinf(THETA), x_scalar, y_scalar);

    // Allocate and arrays shared between ARM and GPU
    SharedArray<float> x_1(N), y_1(N);
    SharedArray<float> x(N), y(N);

    //
    // Compare scalar with QPU output - will not be exact
    //

    INFO("Running kernel 1 (always 1 QPU)");
    auto k = compile(rot3D_1);
    initArrays(x_1, y_1, N);
    k.load(N, cosf(THETA), sinf(THETA), &x_1, &y_1).call();
    compareResults(x_scalar, y_scalar, x_1, y_1, N, "Rot3D 1", false);  // Last param false: do approximate match


    // Compare outputs of the kernel versions.
    // These *should* be exact, because kernel 1 output is compared with kernel 2
    auto k2 = compile(rot3D_2);

    INFO("Running kernel 2 with 1 QPU");
    initArrays(x, y, N);
    k2.load(N, cosf(THETA), sinf(THETA), &x, &y).call();
    compareResults(x_1, y_1, x, y, N, "Rot3D_2");

    INFO("Running kernel 2 with 8 QPUs");
    k2.setNumQPUs(8);
    initArrays(x, y, N);
    k2.load(N, cosf(THETA), sinf(THETA), &x, &y).call();
    compareResults(x_1, y_1, x, y, N, "Rot3D_2 8 QPU's");

    delete [] x_scalar;
    delete [] y_scalar;
  }


  SECTION("Multiple kernel definitions should be possible") {
    if (!Platform::instance().has_vc4) {
      printf("NB: Rot3D kernel unit test not working on v3d, timeout hang (TODO)\n");
      return;
    }

    auto k_1 = compile(rot3D_1);
    SharedArray<float> x_1(N), y_1(N);
    initArrays(x_1, y_1, N);
    k_1.load(N, cosf(THETA), sinf(THETA), &x_1, &y_1).call();

    auto k_2 = compile(rot3D_2);
    SharedArray<float> x_2(N), y_2(N);
    initArrays(x_2, y_2, N);
    k_2.load(N, cosf(THETA), sinf(THETA), &x_2, &y_2).call();

    compareResults(x_1, y_1, x_2, y_2, N, "Rot3D_1 and Rot3D_2 1 QPU");
  }
}
