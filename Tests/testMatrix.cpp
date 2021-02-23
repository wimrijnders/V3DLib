///////////////////////////////////////////////////////////////////////////////
//
// This tests the required components for matrix multiplication
//
///////////////////////////////////////////////////////////////////////////////
#include "catch.hpp"
#include <iostream>
#include <string>
#include <V3DLib.h>
#include "LibSettings.h"
#include "support/support.h"
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

  vec.dot_product(a, tmp2);       comment("check_dotvector end dot_product");
  kernels::set_at(tmp, 0, tmp2);  comment("check_dotvector end kernel set_at");
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
  //k.pretty(true, "obj/test/check_dotvector.txt", false);
  k.load(&b, &a, &result);
  run_kernel(k);

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
      INFO("N: " << N);
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
  int dimension,
  Kernel &k,
  Shared2DArray<float> &a,
  Shared2DArray<float> &result,
  float *a_scalar,
  float *expected) {

  //
  // Multiplication of empty input matrix
  //
  a.fill(0);
  result.fill(-1);
//  k.setNumQPUs(8);
  run_kernel(k);

  for (int r = 0; r < result.rows(); r++) {
    for (int c = 0; c < result.columns(); c++) {
      INFO("Dimension: " << dimension <<", element: (" << r << ", " << c << ")");
      REQUIRE(result[r][c] == 0);
    }
  }

  //
  // Square of input matrix containing all ones
  //
  a.fill(1);
  //dump_array(a.get_parent(), 16);

  result.fill(-1);
//  k.setNumQPUs(8);
  run_kernel(k);

  for (int r = 0; r < result.rows(); r++) {
    for (int c = 0; c < result.columns(); c++) {
      INFO("Dimension: " << dimension <<", element: (" << r << ", " << c << ")");
      REQUIRE(result[r][c] == (float) dimension);
    }
  }

  //
  // Square of unit matrix
  //
  a.make_unit_matrix();
  //dump_array(a.get_parent(), a.columns());

  // Check if indeed unitary
  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      if (r == c) {
        REQUIRE(a[r][c] == 1);
      } else {
        REQUIRE(a[r][c] == 0);
      }
    }
  }

  run_kernel(k);

  for (int r = 0; r < result.rows(); r++) {
    for (int c = 0; c < result.columns(); c++) {
      if (r == c) {
        REQUIRE(result[r][c] == 1);
      } else {
        REQUIRE(result[r][c] == 0);
      }
    }
  }


  //
  // Random values in array
  //
  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      a_scalar[r*a.rows() + c] = a[r][c] = random_float();
    }
  }

  Shared2DArray<float> b(dimension);
  b.copy_transposed(a);

  kernels::square_matrix_mult_scalar(dimension, expected, a_scalar, a_scalar);
  k.load(&result, &a, &b);
  run_kernel(k);

  float precision = 2.5e-4f;  // Empirically determined - the bigger the matrices, the less precise
                              // This value works for 640x640 matrices

  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      REQUIRE(abs(result[r][c] - expected[r*a.rows() + c]) < precision);
    }
  }
}


/**
 *
 */
void test_square_matrix_multiplication(int dimension) {
  //printf ("running test_square_matrix_multiplication() with dim: %d\n", dimension);

  REQUIRE(dimension > 1);
  REQUIRE(dimension % 16 == 0);
  int const SIZE = dimension*dimension;

  Shared2DArray<float> a(dimension);
  Shared2DArray<float> result(dimension);
  result.fill(-1.0f);

  float a_scalar[SIZE];
  for (int i = 0; i < SIZE; i++) {
    a_scalar[i] = 1;
  }

  float expected[SIZE];
  for (int i = 0; i < SIZE; i++) {
    expected[i] = -1;
  }
  kernels::square_matrix_mult_scalar(dimension, expected, a_scalar, a_scalar);

  for (int i = 0; i < SIZE; i++) {
    REQUIRE(expected[i] == (float) dimension);
  }


  // Can't have kernels k and k2 in the same context.
  // One kernel runs but the seconds hangs. Either works fine when run by itself.
  // Unclear why at this stage, but settings separate contexts works.
  {
    INFO("Doing TMU");
    auto k = compile(kernels::matrix_mult_decorator(dimension));
    k.load(&result, &a, &a);
    check_matrix_results(dimension, k, a, result, a_scalar, expected);
  }

  {
    // Do the same thing with DMA (different for vc4 only)
    LibSettings::use_tmu_for_load(false);  // selects DMA
    INFO("Doing DMA");

    auto k2 = compile(kernels::matrix_mult_decorator(dimension));
    k2.pretty(false, "obj/test/Matrix_code_prefetch_dma.txt");  // TODO check on vc4
    k2.load(&result, &a, &a);
    check_matrix_results(dimension, k2, a, result, a_scalar, expected);

    LibSettings::use_tmu_for_load(true);
  }
}



void test_matrix_multiplication (int rows, int inner, int cols, float init_a = 1, float init_b = 1) {
  REQUIRE(rows > 0);
  REQUIRE(inner > 0);
  REQUIRE(cols > 0);
  REQUIRE(inner % 16 == 0);

  Shared2DArray<float> a(rows, inner);
  a.fill(init_a);
  Shared2DArray<float> b(inner, cols);
  b.fill(init_b);

  Shared2DArray<float> result;

  REQUIRE(a.columns() == b.rows());

  auto k = compile(kernels::matrix_mult_decorator(a, b, result));
  result.fill(-1.0f);

  k.load(&result, &a, &b);
  run_kernel(k);
  //dump_array(result.get_parent(), cols_result);

  INFO("rows: " << rows << ", inner: " << inner << ", cols: " << cols);
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      INFO("r: " << r << ", c: " << c);
      REQUIRE(result[r][c] == ((float) inner)*init_a*init_b);
    }
  }
}

}  // anon namespace


TEST_CASE("Test matrix algebra components", "[matrix][comp]") {
  using namespace V3DLib;

  SECTION("Check scalar matrix multiplication") {
    int const N = 16;  // Dimension of square matrix
    int const SIZE = N*N;

    float a[SIZE];
    float b[SIZE];
    float c[SIZE];

    // Init matrices
    for (int i = 0; i < SIZE; i++) { a[i] =  1; }
    for (int i = 0; i < SIZE; i++) { b[i] =  2; }
    for (int i = 0; i < SIZE; i++) { c[i] = -1; }

    kernels::square_matrix_mult_scalar(N, c, a, b);
    
    for (int i = 0; i < SIZE; i++) {
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
    test_dotvector<1>();
    test_dotvector<2>();
    test_dotvector<4>();
    test_dotvector<10>();
  }
}


TEST_CASE("Test matrix algebra", "[matrix][mult]") {
  SECTION("Check matrix multiplication") {
    test_square_matrix_multiplication(16);
    test_square_matrix_multiplication(2*16);
    test_square_matrix_multiplication(5*16);

    // 640x640 matrices, works! If you don't mind waiting for test to complete.
    //test_square_matrix_multiplication(40*16);
  }
}


TEST_CASE("Test matrix algebra with varying sizes", "[matrix][mult][varying]") {
  //Platform::use_main_memory(true);

  SECTION("Check matrix multiplication") {

    // TODO same thing with > 1 QPUs
    //      add warning in matrix mult kernel about this

    test_matrix_multiplication( 1,    16,   1);
    test_matrix_multiplication( 1,  5*16,   1);
    test_matrix_multiplication(10,    16,   5);
    test_matrix_multiplication( 3,  3*16,   3, -1.0f, 2.0f);
    test_matrix_multiplication(65, 10*16, 128,  2.0f, 3.0f);  // Going over the top here with big dimensions
  }

}


///////////////////////////////////////////////////////////////////////////////
// Complex arrays
///////////////////////////////////////////////////////////////////////////////

/**
 * Kernel for unit testing complex dot vectors
 */
template<int const N>
void check_complex_dotvector(Complex::Ptr dst, Complex::Ptr a, Complex::Ptr result) {
  kernels::ComplexDotVector vec(N);
  vec.load(a);
  vec.save(dst);

  Complex tmp(-2, -2);  // Silly value for detection in unit test
  Complex tmp2;

  vec.dot_product(a, tmp2);       comment("check_complex_dotvector end dot_product");
  tmp.set_at(0, tmp2);            comment("check_complex_dotvector end kernel set_at");
  *result = tmp;
}


/**
 * Template parameter N is the number of 16-value blocks in arrays.
 * It also sets the number of registers for a dot vector in the kernel.
 */
template<int const N>
void test_complex_dotvector() {
  Complex::Array a(16*N);

  for (int i = 0; i < (int) a.size(); i++) {
    a[i] = complex(1.0f*((float ) (i + 1)), 0);
  }

  Complex::Array b(16*N);
  b.fill({-1, -1});

  Complex::Array result(16);
  result.fill({-1.0f, -1.0f});

  REQUIRE(a.size() == b.size());

  auto k = compile(check_complex_dotvector<N>);
  k.pretty(true, "obj/test/check_complex_dotvector.txt", false);
  k.load(&b, &a, &result);
  k.call();

  for (int i = 0; i < (int) a.size(); i++) {
    INFO("N: " << N << ", i: " << i);
    REQUIRE(a[i] == b[i]);
  }

  // Calculate and check expected result of dot product
  complex expected(0, 0);
  for (int i = 0; i < (int) a.size(); i++) {
    expected += a[i]*a[i];
  }

  for (int i = 0; i < (int) result.size(); i++) {
    if (i == 0) {
      INFO("N: " << N);
      REQUIRE(result[i] == expected);
    } else {
      REQUIRE(result[i] == complex(-2, -2));
    }
  }

  // Do it again with simpler values for hand calculation
  a.fill({ 1,  0});
  k.call();
  REQUIRE(result[0] == complex(16*N, 0));

  // Do it again with re and im having values
  a.fill({ 1,  2});
  k.call();
  expected = complex(16*N*-3, 16*N*4);
  INFO("result: "   << result[0].dump());
  INFO("expected: " << expected.dump());
  REQUIRE(result[0] == expected);
}


TEST_CASE("Test complex matrix algebra with varying sizes", "[matrix][complex][]") {
//  Platform::use_main_memory(true);

  SECTION("Check correct working complex dotvector") {
    test_complex_dotvector<1>();
    test_complex_dotvector<4>();
    test_complex_dotvector<10>();
  }


//  Platform::use_main_memory(false);
}
