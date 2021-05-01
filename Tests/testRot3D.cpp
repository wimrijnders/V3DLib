#include "doctest.h"
#include <math.h>
#include "Kernels/Rot3D.h"
#include "LibSettings.h"

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
      REQUIRE(y1[i] == y2[i]);
    } else {
      REQUIRE(x1[i] == doctest::Approx(x2[i]).epsilon(0.001));
      REQUIRE(y1[i] == doctest::Approx(y2[i]).epsilon(0.001));
    }
  }
}


// ============================================================================
// The actual tests
// ============================================================================

TEST_CASE("Test working of Rot3D example [rot3d]") {
  // Number of vertices and angle of rotation
  const int N = 16*12*10;  // 1920
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
   *
   * vc4: No problem at all when all kernels loaded in same BO.
   * v3d: rot3D_1 and rot3D_2 can be loaded together, adding the 3-kernels
   *      leads to persistent timeout:
   *
   *    ERROR: v3d_wait_bo() ioctl: Timer expired
   *
   * Running the test multiple times leads to the unit tests hanging (and eventually losing contact with the Pi4.
   *
   * For this reason, separate contexts are used for each kernel. This results in the BO being completely
   * cleared for the next kernel.
   */
  SUBCASE("All kernel versions should return the same") {
    //
    // Run the scalar version
    //

    // Allocate and initialise
    float* x_scalar = new float [N];
    float* y_scalar = new float [N];
    initArrays(x_scalar, y_scalar, N);

    rot3D(N, cosf(THETA), sinf(THETA), x_scalar, y_scalar);

    // Storage for first kernel results
    float* x_1 = new float [N];
    float* y_1 = new float [N];

    {
      // Compare scalar with output kernel 1 - will not be exact
      INFO("Running kernel 1 (always 1 QPU)");
      Float::Array x(N), y(N);
      initArrays(x, y, N);

      auto k = compile(rot3D_1);
      k.load(N, cosf(THETA), sinf(THETA), &x, &y).call();
      compareResults(x_scalar, y_scalar, x, y, N, "Rot3D 1", false);  // Last param false: do approximate match

      // Save results for compare with other kernels 
      for (int i = 0; i < N; ++i) {
        x_1[i] = x[i];
        y_1[i] = y[i];
      }
    }


    //
    // Compare outputs of the kernel versions.
    // The output of kernel 1 output is compared with the output of other kernels 
    // The matches should be exact.
    //

    {
      // This is to check if DMA is still working for load var
      LibSettings::use_tmu_for_load(false);
      Float::Array x(N), y(N);
      auto k = compile(rot3D_1);

      INFO("Running kernel 1 with DMA");
      initArrays(x, y, N);
      k.load(N, cosf(THETA), sinf(THETA), &x, &y).call();
      compareResults(x_1, y_1, x, y, N, "Rot3D_1 DMA");
      LibSettings::use_tmu_for_load(true);
    }

    {
      Float::Array x(N), y(N);
      auto k = compile(rot3D_1a);
      //k.pretty(false, "obj/test/rot3D_1a_v3d.txt", false);

      INFO("Running kernel 1a with 1 QPU");
      initArrays(x, y, N);
      k.load(N, cosf(THETA), sinf(THETA), &x, &y).call();
      compareResults(x_1, y_1, x, y, N, "Rot3D_1a");

      INFO("Running kernel 1a with 8 QPUs");
      k.setNumQPUs(8);
      initArrays(x, y, N);
      k.load(N, cosf(THETA), sinf(THETA), &x, &y).call();
      compareResults(x_1, y_1, x, y, N, "Rot3D_1a");
    }

    {
      Float::Array x(N), y(N);
      auto k = compile(rot3D_2);

      INFO("Running kernel 2 with 1 QPU");
      initArrays(x, y, N);
      k.load(N, cosf(THETA), sinf(THETA), &x, &y).call();
      compareResults(x_1, y_1, x, y, N, "Rot3D_2");

      INFO("Running kernel 2 with 8 QPUs");
      k.setNumQPUs(8);
      initArrays(x, y, N);
      k.load(N, cosf(THETA), sinf(THETA), &x, &y).call();
      compareResults(x_1, y_1, x, y, N, "Rot3D_2 8 QPUs");
    }

    {
      INFO("Running kernel 3 with 1 QPU");
      Float::Array x(N), y(N);
      initArrays(x, y, N);

      auto k = compile(rot3D_3_decorator(N));
      //k.pretty(true, "kernel3_prefetch.txt");
      k.load(cosf(THETA), sinf(THETA), &x, &y).call();
      compareResults(x_1, y_1, x, y, N, "Rot3D_3");
    }

    {
      INFO("Running kernel 3 with 8 QPUs");
      Float::Array x(N), y(N);
      initArrays(x, y, N);

      auto k = compile(rot3D_3_decorator(N, 8));
      k.setNumQPUs(8);
      //k.pretty(true);
      k.load(cosf(THETA), sinf(THETA), &x, &y).call();
      compareResults(x_1, y_1, x, y, N, "Rot3D_3 8 QPUs");
    }

    delete [] x_1;
    delete [] y_1;
    delete [] x_scalar;
    delete [] y_scalar;
  }


  SUBCASE("Multiple kernel definitions in the same context should be possible") {
    auto k_1 = compile(rot3D_1);
    Float::Array x_1(N), y_1(N);
    initArrays(x_1, y_1, N);
    k_1.load(N, cosf(THETA), sinf(THETA), &x_1, &y_1).call();

    // Next kernel intentionally defined in the same shared array as previous kernel
    // That's the goal of this test.
    auto k_2 = compile(rot3D_2);
    Float::Array x_2(N), y_2(N);
    initArrays(x_2, y_2, N);
    k_2.load(N, cosf(THETA), sinf(THETA), &x_2, &y_2).call();

    compareResults(x_1, y_1, x_2, y_2, N, "Rot3D_1 and Rot3D_2 1 QPU");
  }
}
