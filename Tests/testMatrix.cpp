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
#include "Support/Timer.h"
#include "Support/pgm.h"
#include "../Examples/Kernels/Matrix.h"

namespace {
using namespace V3DLib;

// ============================================================================
// Support routines
// ============================================================================


void fill_random(float *arr, int size) {
  for (int n = 0; n < size; n++) {
    arr[n] = random_float();
  }
}


/**
 * Pre: dst properly initialized, matches with src
 */
void copy_array(Float::Array2D &dst, float *src) {
  for (int r = 0; r < dst.rows(); r++) {
    for (int c = 0; c < dst.columns(); c++) {
      dst[r][c] = src[r*dst.columns() + c];
    }
  }
}


void copy_transposed(float *dst, float *src, int rows, int columns) {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < columns; c++) {
      dst[c*rows + r] = src[r*columns + c];
    }
  }
}


void compare_arrays(Float::Array2D &a, float *b) {
  float precision = 2.5e-4f;  // Empirically determined - the bigger the matrices, the less precise
                              // This value works for 640x640 matrices

  for (int r = 0; r < a.rows(); r++) {
    for (int c = 0; c < a.columns(); c++) {
      INFO("r: " << r << ", c: " << c);
      REQUIRE(abs(a[r][c] - b[r*a.columns() + c]) < precision);
    }
  }
}


void compare_arrays(Complex::Array2D &a, Complex::Array2D &b) {
  REQUIRE(a.rows() == b.rows());
  REQUIRE(a.columns() == b.columns());

  for (int r = 0; r < a.rows(); ++r) {
    for (int c = 0; c < a.columns(); ++c) {
      if (c == 0) {
        INFO("(r, c): ( " << r << ", " << c << ")");
        INFO(a[r][c].dump() << " == " << b[r][c].dump());
        REQUIRE(a[r][c] == b[r][c]);
      }
    }
  }
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
void check_sum_kernel(Float::Ptr input, Float::Ptr result) {
  Float val = *input;
  Float sum;

  kernels::rotate_sum(val, sum);
  *result = sum;
}


/**
 * Kernel to test correct working of `set_at`
 */
void check_set_at(Float::Ptr input, Float::Ptr result, Int index) {
  Float a = *input;
  Float b = *result;

  b.set_at(index, a);
  *result = b;
}


/**
 * Kernel for unit testing dot vectors
 */
template<int const N>
void check_dotvector(Float::Ptr dst, Float::Ptr a, Float::Ptr result) {
  kernels::DotVector vec(N);
  vec.load(a);
  vec.save(dst);

  Float tmp = -2;  // Silly value for detection in unit test
  Float tmp2;

  vec.dot_product(a, tmp2);       comment("check_dotvector end dot_product");
  tmp.set_at(0, tmp2);            comment("check_dotvector end kernel set_at");
  *result = tmp;
}


/**
 * Template parameter N is the number of 16-value blocks in arrays.
 * It also sets the number of registers for a dot vector in the kernel.
 */
template<int const N>
void test_dotvector() {
  Float::Array a(16*N);

  for (int i = 0; i < (int) a.size(); i++) {
    a[i] = 1.0f*((float ) (i + 1));
  }

  Float::Array b(16*N);
  b.fill(-1);

  Float::Array result(16);
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
  Float::Array2D &a,
  Float::Array2D &result,
  float *a_scalar,
  float *expected) {

/*
  //
  // Multiplication of empty input matrix
  //
  a.fill(0);
  result.fill(-1);
  run_kernel(k);
  compare_array_scalar(result, 0.0f);

  //
  // Square of input matrix containing all ones
  //
  a.fill(1);
  result.fill(-1);
  run_kernel(k);
  compare_array_scalar(result, (float) dimension);
*/

  //
  // Square of unit matrix
  //
  a.make_unit_matrix();

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
  //k.pretty(true, "obj/test/unitary_matrix_mult_vc4.txt");
  //k.dump_compile_data(true, "obj/test/compile_data_unitary_matrix_mult_vc4.txt");
  //dump_array(a.get_parent(), a.columns());
  //dump_array(result.get_parent(), result.columns());

  for (int r = 0; r < result.rows(); r++) {
    for (int c = 0; c < result.columns(); c++) {
      INFO("rows: " << result.rows() << ", (r,c): (" << r << ", " << c << ")");
      if (r == c) {
        REQUIRE(result[r][c] == 1);
      } else {
        REQUIRE(result[r][c] == 0);
      }
    }
  }


  //
  // Test Random values in array
  // Multiplies array(s) with itself
  //
  fill_random(a_scalar, a.rows()*a.columns());
  copy_array(a, a_scalar);

  float a_transposed[dimension*dimension];
  copy_transposed(a_transposed, a_scalar, dimension, dimension);

  kernels::square_matrix_mult_scalar(dimension, expected, a_scalar, a_transposed);
  k.load(&result, &a, &a);
  run_kernel(k);

  compare_arrays(result, expected);
}


/**
 *
 */
void test_square_matrix_multiplication(int dimension) {
  //printf ("running test_square_matrix_multiplication() with dim: %d\n", dimension);

  REQUIRE(dimension > 1);
  REQUIRE(dimension % 16 == 0);
  int const SIZE = dimension*dimension;

  Float::Array2D a(dimension);
  Float::Array2D result(dimension);
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


void test_matrix_multiplication(int rows, int inner, int cols, float init_a = 1, float init_b = 1, int num_qpus = 1) {
  REQUIRE(rows > 0);
  REQUIRE(inner > 0);
  REQUIRE(cols > 0);
  REQUIRE(inner % 16 == 0);

  Float::Array2D a(rows, inner);
  a.fill(init_a);
  Float::Array2D b(cols, inner);  // Transposed!
  b.fill(init_b);

  Float::Array2D result;

  REQUIRE(a.columns() == b.columns());

  auto k = compile(kernels::matrix_mult_decorator(a, b, result));
  k.setNumQPUs(num_qpus);
  result.fill(-1.0f);

  k.load(&result, &a, &b);
  run_kernel(k);

/*
  // Clear the irrelevant parts of the array before displaying
  for (int r = 0; r < result.rows(); ++r) {
    for (int c = 0; c < result.columns(); ++c) {
      if (r >= rows || c >= cols) {
        result[r][c] = 0;
      }
    }
  }

  std::cout << result.dump() << std::endl;
*/

  INFO("rows: " << rows << ", inner: " << inner << ", cols: " << cols);

  // NOTE: this does not compare the entire results array, just the relevant bit(s)
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
    Float::Array vec(16);
    vec.fill(0.3f);

    Float::Array result(16);
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
    Float::Array vec(16);

    for (int i = 0; i < (int) vec.size(); i++) {
      vec[i] = 1.0f*((float ) (i + 1));
    }

    Float::Array result(16);
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
  SECTION("Check matrix multiplication") {
    test_matrix_multiplication( 1,    16,   1);
    test_matrix_multiplication( 1,  5*16,   1);
    test_matrix_multiplication(10,    16,   5);
    test_matrix_multiplication( 3,  3*16,   3, -1.0f, 2.0f);
    test_matrix_multiplication(65, 10*16, 128,  2.0f, 3.0f);  // Going over the top here with big dimensions

    // same thing with > 1 QPUs
    test_matrix_multiplication( 1,    16,   1,  1   , 1, 8);
    test_matrix_multiplication( 1,  5*16,   1,  1   , 1, 8);
    test_matrix_multiplication(10,    16,   5,  1   , 1, 8);
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
  //k.pretty(true, "obj/test/check_complex_dotvector.txt", false);
  k.load(&b, &a, &result);
  k.call();

  for (int i = 0; i < (int) a.size(); i++) {
    INFO("N: " << N << ", i: " << i);
    INFO("a: " << a.dump());
    INFO("b: " << b.dump());
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


void test_complex_matrix_multiplication(
  int rows, int inner, int cols,
  int num_qpus = 1,
  complex init_a = {1, 0},
  complex init_b = {1, 0}
) {
  REQUIRE(rows > 0);
  REQUIRE(inner > 0);
  REQUIRE(cols > 0);
  REQUIRE(inner % 16 == 0);

  Complex::Array2D a(rows, inner); a.fill(init_a);
  Complex::Array2D b(inner, cols); b.fill(init_b);
  Complex::Array2D result;

  REQUIRE(a.columns() == b.rows());

  auto k = compile(kernels::complex_matrix_mult_decorator(a, b, result));
  k.setNumQPUs(num_qpus);
  result.fill({-1.0f, -1.0f});

  k.load(&result, &a, &b);
  run_kernel(k);
  //dump_array(result.get_parent(), cols_result);

  INFO("rows: " << rows << ", inner: " << inner << ", cols: " << cols);
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      INFO("r: " << r << ", c: " << c);
      INFO("result: " << result[r][c].dump() << ", expected: " << (((float) inner)*init_a*init_b).dump());
      REQUIRE(result[r][c] == ((float) inner)*init_a*init_b);
    }
  }
}


TEST_CASE("Test complex matrix algebra with varying sizes", "[matrix][complex]") {
  SECTION("Check correct working complex dotvector") {
    test_complex_dotvector<1>();
    test_complex_dotvector<4>();
    test_complex_dotvector<10>();
  }

  SECTION("Check complex matrix multiplication") {
    test_complex_matrix_multiplication( 1,    16,   1, 1);
    test_complex_matrix_multiplication( 2,  3*16,   2, 1, {-1.0f, 2.0f});
    test_complex_matrix_multiplication( 2,  3*16,   2, 1, {-1.0f, 2.0f}, { 1.0f, -1.0f });

    // same thing with > 1 QPUs
    test_complex_matrix_multiplication( 1,    16,   1, 8);
    test_complex_matrix_multiplication( 2,  3*16,   2, 8, {-1.0f, 2.0f});
    test_complex_matrix_multiplication( 2,  3*16,   2, 8, {-1.0f, 2.0f}, { 1.0f, -1.0f });
  }


  SECTION("Compare pure real/im complex matrixes with real") {
    int const Dim = 16;
    int const Size = Dim*Dim;

    float scalar[Size];
    fill_random(scalar, Size);

    float scalar_transposed[Size];
    copy_transposed(scalar_transposed, scalar, Dim, Dim);

    float scalar_result[Size];
    kernels::square_matrix_mult_scalar(Dim, scalar_result, scalar, scalar_transposed);

    Complex::Array2D a(Dim);
    Complex::Array2D result(Dim);

    auto k = compile(kernels::complex_matrix_mult_decorator(a, a, result));
    k.load(&result, &a, &a);

    //
    // Compare real only
    //
    copy_array(a.re(), scalar);
    a.im().fill(0.0f);

    k.call();
    compare_arrays(result.re(), scalar_result);
    compare_array_scalar(result.im(), 0.0f);


    //
    // Compare im only - result will be negative of scalar_result
    //
    copy_array(a.im(), scalar);
    a.re().fill(0.0f);

    k.interpret();

    //std::cout << result.dump() << std::endl;

    // Negate result
    for (int r = 0; r < result.rows(); r++) {
      for (int c = 0; c < result.columns(); c++) {
        result[r][c] *= -1;
      }
    }

    compare_arrays(result.re(), scalar_result);
    compare_array_scalar(result.im(), 0.0f);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Discrete Fourier Transform
///////////////////////////////////////////////////////////////////////////////

namespace {

int const DimTest = 128;

void create_test_wavelet(Complex::Array2D &input, int const Dim) {
  REQUIRE(input.columns() == Dim);

  float offset = 0.1f;

  float freq_filter =  0.5f / ((float) Dim);
  float freq1       =  1.0f / ((float) Dim);
  float freq2       = 45.0f / ((float) Dim);

  for (int c = 0; c < Dim; ++c) {
    float filter = functions::sin(freq_filter*((float) c), true);
    float noise = 0.3f *random_float();
    float val1  = 1.0f *functions::sin(freq1*((float) c), true);
    float val2  = 0.75f*functions::sin(freq2*((float) c), true);

    input[0][c] = complex(offset + noise + (filter*filter)*(val1 + val2), 0.0f);
  }
}


void create_dft_matrix(Complex::Array2D &arr) {
  REQUIRE(arr.rows() > 0);
  REQUIRE(arr.rows() % 16 == 0);
  int const Dim = arr.rows();
  REQUIRE(Dim == arr.columns());

  float const precision1 = 1.5e-3f;

  for (int r = 0; r < Dim; ++r) {
    for (int c = 0; c < Dim; ++c) {
      double angle_lib = (2*M_PI*((float) (-r*c)) / Dim);
      complex tmp_lib((float) std::cos(angle_lib), (float) std::sin(angle_lib)); 

      float angle = ((float) (-r*c))/((float) Dim); 
      complex tmp(functions::cos(angle, true), functions::sin(angle, true)); // High-precision is a good idea!
      //std::cout << tmp.dump();

      INFO("r: " << r << ", c: " << c);
      INFO("tmp_lib: " << tmp_lib.dump() << ", tmp: " << tmp.dump());

      REQUIRE(abs(tmp_lib.re() - tmp.re()) < precision1);
      REQUIRE(abs(tmp_lib.im() - tmp.im()) < precision1);

      arr[r][c] = tmp;
    }
  }
}


/**
 * Create some visual output for DFT matrices
 */
void output_dft(Complex::Array2D &input, Complex::Array2D &result, char const *file_prefix) {
  assert(file_prefix != nullptr);
  int const Dim = input.columns();

  {
    float real_input[Dim];
    for (int c = 0; c < Dim; ++c) {
      real_input[c] = input[0][c].re();
    }

    std::string filename = "obj/test/";
    filename << file_prefix << "_input.pgm";

    PGM pgm(Dim, 100);
    pgm.plot(real_input, Dim)
       .save(filename.c_str());
  }

  {
    float real_result[Dim];
    for (int c = 0; c < Dim; ++c) {
      real_result[c] = result[0][c].magnitude();
    }

    std::string filename = "obj/test/";
    filename << file_prefix << "_result.pgm";

    PGM pgm(Dim, 100);
    pgm.plot(real_result, Dim)
       .save(filename.c_str());
  }
}


/**
 * @return  true if all kernels compiled something,
 *          false if any kernel failed compilation for vc4 as well as v3d
 */ 
bool compare_dfts(int Dim, std::vector<int> num_qpus, bool do_profiling) {
  assert(Dim > 0 && (Dim % 16 == 0));

  //
  // Support Stuff
  //
  bool const ShowCompile = false;
  CompileFor for_platform = (Platform::has_vc4())?CompileFor::VC4:CompileFor::V3D;

  struct out_data {
    out_data(
      std::string const &in_label,
      std::string const &in_timer,
      int in_Dim,
      int in_num_qpus
      ) : label(in_label), Dim(in_Dim), num_qpus(in_num_qpus), timer(in_timer) {}

    std::string str() const {
      std::string ret;

      ret << tabbed(2, num_qpus) << ", " << tabbed(3, Dim) << ", "
          << tabbed(16, label) << ", " << timer;

      return ret;
    }

    std::string label;
    int Dim;
    int num_qpus;
    std::string timer;
  };

  std::vector<out_data> output;

  auto add_compile = [ShowCompile, &output] (std::string const &label, Timer &timer, int Dim, int num_qpus) {
    if (!ShowCompile) return; 

    std::string str;
    str << "\"compile " << label << "\"";

    output << out_data(str, timer.end(false), Dim, num_qpus);
  };

  auto add_call = [&output] (std::string const &label, Timer &timer, int Dim, int num_qpus) {
    std::string str;
    str << "\"" << label << "\"";

    output << out_data(str, timer.end(false), Dim, num_qpus);
  };

  //
  // Initialize shared arrays
  //
  Complex::Array2D input(1, Dim);  // Create input; remember, transposed!
  create_test_wavelet(input, Dim);

  Float::Array input_float(Dim);
  for (int i = 0; i < input.re().columns(); ++i) {
    input_float[i] = input.re()[0][i];
  }

  // Prepare DFT matrix
  Complex::Array2D dft_matrix(Dim);
  create_dft_matrix(dft_matrix);

  Complex::Array2D result_mult;  // Will be Dimx1
  Complex::Array2D result_complex;
  Complex::Array2D result_float;

  //
  // Run the kernels
  //
  int compiled = 0;

  {
    std::string label = "matrix mult";

    // Do regular complex matrix multiplication
    // In this call, the matrix and input are switched.
    // This is slightly more efficient and should not affect the result
    Timer timer1;
    auto k = compile(kernels::complex_matrix_mult_decorator(input, dft_matrix, result_mult), for_platform);
    add_compile(label, timer1, Dim, 0);

    if (!k.has_errors()) compiled += 1;

    if (!k.has_errors()) {
      k.load(&result_mult, &input, &dft_matrix);

      for (auto num : num_qpus) {
        k.setNumQPUs(num);
        Timer timer;
        k.call();
        add_call(label, timer, Dim, num);
      }
    }
  }

  {
    std::string label = "inline complex";

    Timer timer1;
    auto k = compile(kernels::dft_inline_decorator(input, result_complex), for_platform);
    add_compile(label, timer1, Dim, 0);

    if (!k.has_errors()) compiled += 2;

    if (!k.has_errors()) {
      k.load(&result_complex, &input);

      for (auto num : num_qpus) {
        k.setNumQPUs(num);
        Timer timer;
        k.call();
        add_call(label, timer, Dim, num);
      }

      output_dft(input, result_complex, "dft_inline_complex");
    }
  }

  {
    std::string label = "inline float";

    Timer timer1;
    auto k = compile(kernels::dft_inline_decorator(input_float, result_float), for_platform);
    add_compile(label, timer1, Dim, 0);

    if (!k.has_errors()) compiled += 4;

    if (!k.has_errors()) {
      k.load(&result_float, &input_float);

      for (auto num : num_qpus) {
        k.setNumQPUs(num);
        Timer timer;
        k.call();
        add_call(label, timer, Dim, num);
      }

      output_dft(input, result_complex, "dft_inline_float");
    }
  }

  //std::cout << result_mult.dump() << std::endl;
  //std::cout << result_complex.dump() << std::endl;

  if (do_profiling) {
    std::string platform = (Platform::has_vc4())?"vc4":"v3d";

    for (int i = 0; i < (int) output.size(); ++i) {
      std::cout << platform << "     , " << output[i].str() << "\n";
    }
  } else {
    REQUIRE(compiled == 7);  // All bits for all kernels should be set
    compare_arrays(result_mult, result_complex);
    compare_arrays(result_mult, result_float);
  }

  //std::cout << "compiled: " << compiled;
  return (compiled != 0);
}

}  // anon namespace


TEST_CASE("Discrete Fourier Transform", "[matrix][dft]") {

  /**
   * Check out how DFT with a matrix looks like
   * Turns out that the precision is pretty lousy - still usable, though.
   */
  SECTION("Check DFT matrix") {
    int const DimShift = 4;
    int const Dim = 1 << DimShift;

    Complex::Array2D dft_matrix(Dim);
    create_dft_matrix(dft_matrix);
    //std::cout << dft_matrix.dump();

    // Tranpose should be equal to self
    for (int r = 0; r < Dim; ++r) {
      for (int c = 0; c < Dim; ++c) {
        REQUIRE(dft_matrix[r][c] == dft_matrix[c][r]); 
      }
    }


    //
    // Multiplying with (transposed) conjugate should give DimxI (I: identity matrix)
    //
    Complex::Array2D dft_conjugate(Dim);
    for (int r = 0; r < Dim; ++r) {
      for (int c = 0; c < Dim; ++c) {
        dft_conjugate[r][c] = dft_matrix[r][c].conjugate();  // Don't transpose!
      }
    }

    Complex::Array2D result;

    auto k = compile(kernels::complex_matrix_mult_decorator(dft_conjugate, dft_matrix, result));
    result.fill({-1, -1});
    k.load(&result, &dft_conjugate, &dft_matrix).emu();

    float const precision2 = 2e-2f;

    for (int r = 0; r < Dim; ++r) {
      for (int c = 0; c < Dim; ++c) {

        float test = 0.0f;
        if (r == c) {
          test = (float) Dim;
        }

        INFO("r: " << r << ", c: " << c);
        REQUIRE(abs(result[r][c].re() - test) < precision2);
        REQUIRE(abs(result[r][c].im() - 0.0f) < precision2);
      }
    }
  }


  SECTION("Check DFT using standard matrix multiplication") {
    int const Dim = DimTest;

    Complex::Array2D input(1, Dim);  // Create input; remember, transposed!
    create_test_wavelet(input, Dim);

    // Prepare DFT matrix
    Complex::Array2D dft_matrix(Dim);
    create_dft_matrix(dft_matrix);

    Complex::Array2D result(1, Dim);

    {
      Complex::Array2D result_tmp;  // Will be Dimx16, columns padded to 16 and only 1st relevant
      auto k = compile(kernels::complex_matrix_mult_decorator(dft_matrix, input, result_tmp));

      //std::cout << "result dimensions: (" << result_tmp.rows() << ", " << result_tmp.columns() << ")" << std::endl;

      k.setNumQPUs(8);  // Running with multi-QPU gives very limited performance improvement
      result.fill({-1, -1});
      k.load(&result_tmp, &dft_matrix, &input);
      k.call();

      // Columns are padded to multiples of 16, only the first column is relevant
      // Translate to better form
      for (int c = 0; c < Dim; ++c) {
        result[0][c] = result_tmp[c][0];
      }
    }


    // Switching input and matrix around should have same result, but transposed compared to previous
    // No need to transpose dft_matrix due to symmetry
    Complex::Array2D result_switched;  // Will be Dimx1

    {
      auto k = compile(kernels::complex_matrix_mult_decorator(input, dft_matrix, result_switched));

      k.setNumQPUs(8);  // Running with multi-QPU gives very limited performance improvement
      k.load(&result_switched, &input, &dft_matrix);

      k.interpret();
    }

    //std::cout << result.dump() << std::endl;
    //std::cout << result_switched.dump() << std::endl;

    REQUIRE(result_switched.columns() == Dim);
    compare_arrays(result, result_switched);

    output_dft(input, result_switched, "dft");
  }
}


TEST_CASE("Discrete Fourier Transform tmp", "[matrix][dft2]") {
  SECTION("Check DFT with inline sin/cos") {
    int const Dim = 16*2;  // max vc4: 16*4. Max v3d is higher, at least 64*8

    Complex::Array2D input(1, Dim);  // Create input; remember, transposed!
    create_test_wavelet(input, Dim);

    Complex::Array2D result;

    Timer timer1("DFT compile time");
    auto k = compile(kernels::dft_inline_decorator(input, result));
    timer1.end();
/*
    k.pretty(true,  "obj/test/dft_inline_vc4.txt", false);
    k.dump_compile_data(true, "obj/test/dft_compile_data_vc4.txt");
    k.pretty(false, "obj/test/dft_inline_v3d.txt", false);
*/
    std::cout << k.compile_info() << std::endl;

    k.load(&result, &input);
    k.call();

    output_dft(input, result, "dft_inline");
  }

  SECTION("All DFT calculations should return the same") {
    bool do_profiling = true;

    if (!do_profiling) {
      // Following is enough for the unit test
      for (int N = 1; N < 4; ++N) {
        bool no_errors = compare_dfts(16*N, {1}, false);
        REQUIRE(no_errors);
      }
    } else {
      // Profiling: try all sizes until compilation fails
      std::cout << "DFT compare\n"
                << "Platform,#QPU,DIM, Label           , Time\n";

      if (Platform::has_vc4()) {
        REQUIRE(false);  // TODO
      } else {
        int N = 1;

        bool can_continue = true;
        while (can_continue) {
          can_continue = compare_dfts(16*N, {1, 8}, true);
          N += 4;
          if (N > 4) break;
        }
      }
    }
  }
}
