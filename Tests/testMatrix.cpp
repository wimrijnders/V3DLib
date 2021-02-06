///////////////////////////////////////////////////////////////////////////////
//
// This tests the required components for matrix multiplication
//
///////////////////////////////////////////////////////////////////////////////
#include "catch.hpp"
#include <string>
#include <V3DLib.h>
#include "Support/basics.h"
#include "support/support.h"
#include "../Examples/Kernels/Matrix.h"

namespace {
using namespace V3DLib;

// ============================================================================
// Support routines
// ============================================================================

/**
 * Convenience method to make switching run modes easier
 */
template<typename Kernel>
void run_kernel(Kernel &k) {
  //k.interpret();
  //k.emu();
  k.call();  // k.qpu() is the intention, but is not compiled on non-arm
}


/**
 * Copy values from square array `a` to array `b`, tranposing the array in the process
 */
template<typename Arr>
void matrix_copy_transposed(Arr &b, Arr &a, int dim) {
  for (int x = 0; x < dim; x++) {
    for (int y = 0; y < dim; y++) {
      b[y + dim*x] = a[x + dim*y];
    }
  }
}


/**
 * Kernel to test correct working of `rotate_sum`
 */
void check_sum_kernel(Ptr<Float> input, Ptr<Float> result) {
  Float val = *input;
  Float sum;

  kernels::rotate_sum(val, sum);
  *result = sum;
}


/**
 * Kernel to test correct working of `set_at`
 */
void check_set_at(Ptr<Float> input, Ptr<Float> result, Int index) {
  Float a = *input;
  Float b = *result;

  kernels::set_at(b, index, a);

  *result = b;
}


/**
 * Kernel for unit testing dot vectors
 */
template<int const N>
void check_dotvector(Ptr<Float> dst, Ptr<Float> a, Ptr<Float> result) {
  kernels::DotVector vec(N);
  vec.load(a);
  vec.save(dst);

  Float tmp = -2;  // Silly value for detection in unit test
  Float tmp2;

  vec.dot_product(a, tmp2);
  kernels::set_at(tmp, 0, tmp2);
  *result = tmp;
}


/**
 * Template parameter N is the number of 16-value blocks in arrays.
 * It also sets the number of registers for a dot vector in the kernel.
 */
template<int const N>
void test_dotvector() {
  SharedArray<float> a(16*N);

  for (int i = 0; i < (int) a.size(); i++) {
    a[i] = 1.0f*((float ) (i + 1));
  }

  SharedArray<float> b(16*N);
  b.fill(-1);

  SharedArray<float> result(16);
  result.fill(-1.0f);

  REQUIRE(a.size() == b.size());

  auto k = compile(check_dotvector<N>);
  k.load(&b, &a, &result);
  run_kernel(k);
  //k.emu();

  if (N <= 2) {
    dump_array(result, 16);
  }

  for (int i = 0; i < (int) a.size(); i++) {
    INFO("N: " << N << ", i: " << i);
    REQUIRE(a[i] == b[i]);
  }

  // Calculate and check expected result of dot product
  float expected = 0;
  for (int i = 0; i < (int) a.size(); i++) {
    expected += a[i]*a[i];
  }

  for (int i = 0; i < (int) result.size(); i++) {
    if (i == 0) {
      INFO("result[0]: " << result[i]);
      REQUIRE(result[i] == expected);
    } else {
      REQUIRE(result[i] == -2);
    }
  }

  // Do it again with simpler values for hand calculation
  a.fill(1);
  k.load(&b, &a, &result);
  run_kernel(k);
  REQUIRE(result[0] == 16*N);
}


template<typename Kernel>
void check_matrix_results(
  int SIZE,
  int dimension,
  Kernel &k,
  SharedArray<float> &a,
  SharedArray<float> &result,
  float *a_scalar,
  float *expected) {

  //
  // Multiplication of empty input matrix
  //
  a.fill(0);
  result.fill(-1);
  run_kernel(k);

  for (int i = 0; i < SIZE; i++) {
    INFO("Dimension: " << dimension <<", i: " << i);
    REQUIRE(result[i] == 0);
  }

  //
  // Square of input matrix containing all ones
  //
  a.fill(1);
  result.fill(-1);
  run_kernel(k);

  for (int i = 0; i < SIZE; i++) {
    INFO("Index " << i << ", [x,y] = [" << (i % dimension) << ", " << (i / dimension) << "]");
    REQUIRE(result[i] == (float) dimension);
  }

  //
  // Square of unit matrix
  //
  a.fill(0);
  for (int i = 0; i < dimension; i++) {
    a[i + dimension*i] = 1;
  }

  run_kernel(k);

  for (int x = 0; x < dimension; x++) {
    for (int y = 0; y < dimension; y++) {
      if (x == y) {
        REQUIRE(result[x + dimension*y] == 1);
      } else {
        REQUIRE(result[x + dimension*y] == 0);
      }
    }
  }


  //
  // Random values in array
  //
  for (int i = 0; i < SIZE; i++) {
    a_scalar[i] = a[i] = random_float();
  }

  SharedArray<float> b(SIZE);
  matrix_copy_transposed(b, a, dimension);

  kernels::matrix_mult_scalar(dimension, expected, a_scalar, a_scalar);

  k.load(&result, &a, &b);

  //printf("Running kernel\n");
  run_kernel(k);
  //printf("Done running kernel\n");

  float precision = 2.5e-4f;  // Empirically determined - the bigger the matrices, the less precise
                              // This value works for 640x640 matrices

  for (int i = 0; i < SIZE; i++) {
    REQUIRE(abs(result[i] - expected[i]) < precision);
  }
}


/**
 *
 */
void test_matrix_multiplication(int dimension) {
  //printf ("running test_matrix_multiplication() with dim: %d\n", dimension);

  REQUIRE(dimension > 1);
  REQUIRE(dimension % 16 == 0);
  int const SIZE = dimension*dimension;

  SharedArray<float> a(SIZE);
  SharedArray<float> result(SIZE);
  result.fill(-1.0f);

  float a_scalar[SIZE];
  for (int i = 0; i < SIZE; i++) {
    a_scalar[i] = 1;
  }

  float expected[SIZE];
  for (int i = 0; i < SIZE; i++) {
    expected[i] = -1;
  }
  kernels::matrix_mult_scalar(dimension, expected, a_scalar, a_scalar);

  for (int i = 0; i < SIZE; i++) {
    REQUIRE(expected[i] == (float) dimension);
  }


  // Can't have kernels k and k2 in the same context.
  // One kernel runs but the seconds hangs. Either works fine when run by itself.
  // Unclear why at this stage, but settings separate contexts works.
  {
    auto k = compile(kernels::matrix_mult_decorator(dimension));
    k.load(&result, &a, &a);
    k.pretty(true,  "obj/test/Matrix_code_vc4.txt");
    k.pretty(false, "obj/test/Matrix_code.txt");
    check_matrix_results(SIZE, dimension, k, a, result, a_scalar, expected);
  }

  {
    // Do the same thing with TMU (different for vc4 only)
    auto k2 = compile(kernels::matrix_mult_decorator(dimension, kernels::USE_TMU));
    k2.pretty(false, "obj/test/Matrix_code_prefetch.txt");
    k2.load(&result, &a, &a);
    check_matrix_results(SIZE, dimension, k2, a, result, a_scalar, expected);
  }
}

}  // anon namespace


TEST_CASE("Test matrix algebra components", "[matrix][comp]") {
  using namespace V3DLib;
  //Platform::use_main_memory(true);  // Run only interpreter and emulator for now


  SECTION("Check scalar matrix multiplication") {
    const int N = 16;  // Dimension of square matrix

    float a[N*N];
    float b[N*N];
    float c[N*N];

    // Init matrices
    for (int i = 0; i < N*N; i++) { a[i] =  1; }
    for (int i = 0; i < N*N; i++) { b[i] =  2; }
    for (int i = 0; i < N*N; i++) { c[i] = -1; }

    kernels::matrix_mult_scalar(N, c, a, b);
    
    for (int i = 0; i < N*N; i++) {
      REQUIRE(c[i] == 32);
    }
  }


  SECTION("Check rotate sum") {
    SharedArray<float> vec(16);
    vec.fill(0.3f);

    SharedArray<float> result(16);
    result.fill(-1);

    auto k = compile(check_sum_kernel);
    k.load(&vec, &result);
    run_kernel(k);

    float precision = 0.0f;
    if (Platform::has_vc4()) {
      precision = 1e-5f;
    }

    REQUIRE(abs(result[0] - 4.8f) <= precision);

    for (int i = 0; i < (int) vec.size(); i++) {
      vec[i] = 0.1f*((float ) (i + 1));
    }

    k.load(&vec, &result);
    run_kernel(k);
    REQUIRE(abs(result[0] - 0.1f*(16*17/2)) <= precision);
  }


  SECTION("Check setting single vector element") {
    SharedArray<float> vec(16);

    for (int i = 0; i < (int) vec.size(); i++) {
      vec[i] = 1.0f*((float ) (i + 1));
    }

    SharedArray<float> result(16);
    result.fill(-1);

    auto k = compile(check_set_at);
    k.load(&vec, &result, 0);
    run_kernel(k);

    for (int i = 0; i < (int) result.size(); i++) {
      if (i == 0) {
        REQUIRE(result[i] == vec[i]);
      } else {
        REQUIRE(result[i] == -1);
      }
    }

    k.load(&vec, &result, 7);
    run_kernel(k);

    for (int i = 0; i < (int) result.size(); i++) {
      if (i == 0 || i == 7) {
        REQUIRE(result[i] == vec[i]);
      } else {
        REQUIRE(result[i] == -1);
      }
    }
  }


  SECTION("Check correct working dotvector") {
    test_dotvector<2>();
    test_dotvector<4>();
    test_dotvector<10>();
  }

  //Platform::use_main_memory(false);
}


TEST_CASE("Test matrix algebra", "[matrix][mult]") {
  //Platform::use_main_memory(true);

  SECTION("Check matrix multiplication") {
    test_matrix_multiplication(16);
    test_matrix_multiplication(2*16);
    test_matrix_multiplication(5*16);
    //test_matrix_multiplication(40*16);  // 640x640 matrices, works! If you don't mind waiting for test to complete.
  }

  //Platform::use_main_memory(false);
}
