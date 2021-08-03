///////////////////////////////////////////////////////////////////////////////
//
// This tests the required components for matrix multiplication
//
///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include <V3DLib.h>
#include "LibSettings.h"
#include "support/support.h"
#include "Support/basics.h"
#include "Kernels/ComplexDotVector.h"
#include "Kernels/Matrix.h"
#include "support/matrix_support.h"
#include "support/ProfileOutput.h"
#include "Support/Timer.h"

namespace {
using namespace V3DLib;

// ============================================================================
// Support routines
// ============================================================================

void prepare_random(Float::Array2D &a, std::vector<float> &expected, int dimension) {
  int const SIZE = dimension*dimension;
  std::vector<float> a_scalar(SIZE);
  std::vector<float> a_transposed(SIZE);
  expected.resize(SIZE);

  fill_random(a_scalar);
  copy_array(a, a_scalar);
  copy_transposed(a_transposed, a_scalar, dimension, dimension);
  kernels::matrix_mult_scalar(dimension, expected.data(), a_scalar.data(), a_transposed.data());
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
  k.load(&b, &a, &result);
  k.call();

  for (int i = 0; i < (int) a.size(); i++) {
    INFO("N: " << N << ", i: " << i);
    INFO("a: " << a.dump());
    INFO("b: " << b.dump());
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
      REQUIRE(result[i] == -2.0f);
    }
  }

  // Do it again with simpler values for hand calculation
  a.fill(1);
  k.load(&b, &a, &result);
  k.call();
  REQUIRE(result[0] == (float) (16*N));
}


template<typename Kernel>
void check_matrix_results(
  int dimension,
  Kernel &k,
  Float::Array2D &a,
  Float::Array2D &result,
  float *a_scalar,
  float *expected) {

  //
  // Multiplication of empty input matrix
  //
  a.fill(0);
  result.fill(-1);
  k.call();
  compare_array_scalar(result, 0.0f);

  //
  // Square of input matrix containing all ones
  //
  a.fill(1);
  result.fill(-1);
  k.call();
  compare_array_scalar(result, (float) dimension);

  //
  // Square of unit matrix
  //
  a.make_unit_matrix();
  check_unitary(a);

  k.call();
  check_unitary(result);

  //
  // Test Random values in array
  // Multiplies array(s) with itself
  //
  fill_random(a_scalar, a.rows()*a.columns());
  copy_array(a, a_scalar);

  float a_transposed[dimension*dimension];
  copy_transposed(a_transposed, a_scalar, dimension, dimension);

  kernels::matrix_mult_scalar(dimension, expected, a_scalar, a_transposed);
  k.load(&result, &a, &a);
  k.call();

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
  kernels::matrix_mult_scalar(dimension, expected, a_scalar, a_scalar);

  for (int i = 0; i < SIZE; i++) {
    REQUIRE(expected[i] == (float) dimension);
  }


  INFO("Doing TMU");
  auto k = compile(kernels::matrix_mult_decorator(dimension));
  k.load(&result, &a, &a);
  check_matrix_results(dimension, k, a, result, a_scalar, expected);

  // Do the same thing with DMA (different for vc4 only)
  LibSettings::use_tmu_for_load(false);  // selects DMA
  INFO("Doing DMA");

  auto k2 = compile(kernels::matrix_mult_decorator(dimension));
  k2.load(&result, &a, &a);
  check_matrix_results(dimension, k2, a, result, a_scalar, expected);

  LibSettings::use_tmu_for_load(true);
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

  INFO("rows: " << rows << ", inner: " << inner << ", cols: " << cols << ", num QPUs: " << num_qpus);

  auto k = compile(kernels::matrix_mult_decorator(a, b, result));
  REQUIRE(!k.has_errors());

  k.setNumQPUs(num_qpus);
  result.fill(-1.0f);

  k.load(&result, &a, &b);
  k.call();

  // NOTE: this does not compare the entire results array, just the relevant bit(s)
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      INFO("r: " << r << ", c: " << c);
      INFO("result array:\n" << result.dump());
      REQUIRE(result[r][c] == ((float) inner)*init_a*init_b);
    }
  }
}

}  // anon namespace


TEST_CASE("Test matrix algebra components [matrix][comp]") {
  using namespace V3DLib;

  SUBCASE("Check scalar matrix multiplication") {
    int const N = 16;  // Dimension of square matrix
    int const SIZE = N*N;

    float a[SIZE];
    float b[SIZE];
    float c[SIZE];

    // Init matrices
    for (int i = 0; i < SIZE; i++) { a[i] =  1; }
    for (int i = 0; i < SIZE; i++) { b[i] =  2; }
    for (int i = 0; i < SIZE; i++) { c[i] = -1; }

    kernels::matrix_mult_scalar(N, c, a, b);
    
    for (int i = 0; i < SIZE; i++) {
      REQUIRE(c[i] == (float) 32);
    }
  }


  SUBCASE("Check rotate sum") {
    Float::Array vec(16);
    vec.fill(0.3f);

    Float::Array result(16);
    result.fill(-1);

    auto k = compile(check_sum_kernel);
    k.load(&vec, &result);
    k.call();

    float precision = 0.0f;
    if (Platform::has_vc4()) {
      precision = 1e-5f;
    }

    REQUIRE(abs(result[0] - 4.8f) <= precision);

    for (int i = 0; i < (int) vec.size(); i++) {
      vec[i] = 0.1f*((float ) (i + 1));
    }

    k.load(&vec, &result);
    k.call();
    REQUIRE(abs(result[0] - 0.1f*(16*17/2)) <= precision);
  }


  SUBCASE("Check setting single vector element") {
    Float::Array vec(16);

    for (int i = 0; i < (int) vec.size(); i++) {
      vec[i] = 1.0f*((float ) (i + 1));
    }

    Float::Array result(16);
    result.fill(-1);

    auto k = compile(check_set_at);
    k.load(&vec, &result, 0);
    k.call();

    for (int i = 0; i < (int) result.size(); i++) {
      if (i == 0) {
        REQUIRE(result[i] == vec[i]);
      } else {
        REQUIRE(result[i] == -1.0f);
      }
    }

    k.load(&vec, &result, 7);
    k.call();

    for (int i = 0; i < (int) result.size(); i++) {
      if (i == 0 || i == 7) {
        REQUIRE(result[i] == vec[i]);
      } else {
        REQUIRE(result[i] == -1.0f);
      }
    }
  }


  SUBCASE("Check correct working dotvector") {
    test_dotvector<1>();
    test_dotvector<2>();
    test_dotvector<4>();
    test_dotvector<10>();
  }
}


TEST_CASE("Test matrix algebra [matrix][mult]") {
  SUBCASE("Check matrix multiplication") {
    test_square_matrix_multiplication(16);
    test_square_matrix_multiplication(2*16);
    test_square_matrix_multiplication(5*16);
    //test_square_matrix_multiplication(40*16); // Works, if you don't mind waiting for test to complete.
  }
}


TEST_CASE("Test matrix algebra with varying sizes [matrix][mult][varying]") {
  SUBCASE("Check matrix multiplication") {
    auto test = [] (int num_qpus) {
      test_matrix_multiplication( 1,    16,   1,  1   , 1   , num_qpus);
      test_matrix_multiplication( 1,  5*16,   1,  1   , 1   , num_qpus);
      test_matrix_multiplication(10,    16,   5,  1   , 1   , num_qpus);
      test_matrix_multiplication( 3,  3*16,   3, -1.0f, 2.0f, num_qpus);
      test_matrix_multiplication(65, 10*16, 128,  2.0f, 3.0f, num_qpus);  // Going over the top with big dimensions
    };

    test(1);
    test(8);  // same thing with > 1 QPUs
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
  k.pretty(false, "check_complex_dotvector.txt");
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


void check_complex_matrix_multiplication(
  int rows, int inner, int cols,
  Complex::Array2D const &result,
  complex expected
) {
  INFO("rows: " << rows << ", inner: " << inner << ", cols: " << cols);
  INFO("result rows: " << result.rows() << ", result columns: " << result.columns());

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      INFO("r: " << r << ", c: " << c);
      INFO("result array:\n" << result.dump());
      INFO("result: " << result[r][c].dump() << ", expected: " << (((float) inner)*expected).dump());
      REQUIRE(result[r][c] == ((float) inner)*expected);
    }
  }
}


/**
 * Test multiplication with all values in input arrays the same.
 * This is just to make result checking simple.
 */
void test_complex_matrix_multiplication(
  int rows, int inner, int cols,
  int num_qpus = 1,
  int num_blocks = 1,
  complex init_a = {1, 0},
  complex init_b = {1, 0},
  CallType call_type = CALL
) {
  REQUIRE(rows > 0);
  REQUIRE(inner > 0);
  REQUIRE(cols > 0);
  REQUIRE(inner % 16 == 0);

  Complex::Array2D a(rows, inner); a.fill(init_a);
  Complex::Array2D b(cols, inner); b.fill(init_b);  // Remember, b transposed
  Complex::Array2D result;

  //
  // Test using decorator - this does not use num_blocks
  //
  INFO("Decorator");
  auto k = compile(kernels::matrix_mult_decorator(a, b, result));
  k.pretty(true, "mult_complex_vc4.txt");
  k.setNumQPUs(num_qpus);
  result.fill({-1.0f, -1.0f});

  k.load(&result, &a, &b);

  switch(call_type) {
    case CALL:      k.call();      break;
    case INTERPRET: k.interpret(); break;
    case EMULATE:   k.emu();       break;
  }

  INFO("num QPUs:" << num_qpus << ", num blocks: " << num_blocks);
  check_complex_matrix_multiplication(rows, inner, cols, result, init_a*init_b);

  //
  // Test using class Matrix
  //
  INFO("Matrix");
  Matrix m(a, b);
  //m.compile();  // Not really required, added to trigger debug statements for recompile
  m.setNumQPUs(num_qpus);
  m.num_blocks(num_blocks);
  m.call(call_type);
  check_complex_matrix_multiplication(rows, inner, cols, m.result(), init_a*init_b);
}


TEST_CASE("Test complex matrix algebra with varying sizes [matrix][complex]") {
  SUBCASE("Check correct working complex dotvector") {
    test_complex_dotvector<1>();
    test_complex_dotvector<4>();
    test_complex_dotvector<10>();
  }

  SUBCASE("Check complex matrix multiplication") {
    auto test = [] (int num_qpus, int num_blocks = 1) {
      // num_blocks factor for inner dimension is there to ensure block sizes are always valid
      test_complex_matrix_multiplication(  1,    16*num_blocks,  1, num_qpus, num_blocks);
      test_complex_matrix_multiplication(  2,  2*16*num_blocks,  2, num_qpus, num_blocks, {-1.0f, 2.0f});
      test_complex_matrix_multiplication(  2,  3*16*num_blocks,  2, num_qpus, num_blocks, {-1.0f, 2.0f}, { 1.0f, -1.0f });
      test_complex_matrix_multiplication( 16,  2*16*num_blocks, 16, num_qpus, num_blocks);
      test_complex_matrix_multiplication( 14,  2*16*num_blocks, 17, num_qpus, num_blocks);
    };

    test(1);
    test(8);
    test(1, 2);
    test(8, 2);
  }


  SUBCASE("Compare pure real/im complex matrixes with real") {
    int const Dim = 16*2;
    int const Size = Dim*Dim;

    float scalar[Size];
    fill_random(scalar, Size);

    float scalar_result[Size];
    {
      float scalar_transposed[Size];
      copy_transposed(scalar_transposed, scalar, Dim, Dim);
      kernels::matrix_mult_scalar(Dim, scalar_result, scalar, scalar_transposed);
    }

    Complex::Array2D a(Dim);
    Complex::Array2D result(Dim);

    auto k = compile(kernels::matrix_mult_decorator(a, a, result));
    k.load(&result, &a, &a);
    k.pretty(false, "obj/test/real_im_v3d.txt");

    //
    // Compare real only
    //
    copy_array(a.re(), scalar);
    a.im().fill(0.0f);
    {
      Timer("Real only", true);
      k.call();
    }
    compare_arrays(result.re(), scalar_result);
    compare_array_scalar(result.im(), 0.0f);


    //
    // Compare im only - result will be negative of scalar_result
    //
    copy_array(a.im(), scalar);
    a.re().fill(0.0f);
    {
      Timer("Im only", true);
      k.call();
    }

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


namespace {

bool profile_block_mult(int dimension) {
  using V3DLib::Timer;

  REQUIRE(dimension > 0);
  REQUIRE(dimension % 2*16 == 0);
  REQUIRE(ProfileOutput::num_iterations > 0);

  ProfileOutput profile_output;
  profile_output.show_compile(false);
  //profile_output.use_max_qpus(true);
  //profile_output.use_single_qpu(true);

  // Prepare input and expected result
  Float::Array2D a(dimension);
  std::vector<float> expected;
  prepare_random(a, expected, dimension);

  //
  // Run the kernels
  //
  int compiled = 0;

  Matrix m(a, a);

  // Running with 1 block is practically identical to a full mult, only extra is offsets passed.
  std::string label1 = "Mult 1 block";
  m.num_blocks(1);

  if (true) {
    Timer timer1;
    m.compile();
    profile_output.add_compile(label1, timer1.end(false), dimension);

    if (!m.has_errors()) {
      compiled += 1;

      profile_output.run(dimension, label1, [&m] (int numQPUs) {
        m.setNumQPUs(numQPUs);
        m.call();
      });

      INFO("Compare 1 block result with expected");
      INFO("num QPUs: " << m.numQPUs());
      compare_arrays(m.result(), expected, 1e-3f);   // just using same precision as full mult
    }
  }

  std::string label2 = "Mult 2 blocks";
  m.num_blocks(2);

  Timer timer2;
  m.compile();
  profile_output.add_compile(label2, timer2.end(false), dimension);

  if (!m.has_errors()) {
    compiled += 2;

    profile_output.run(dimension, label2, [&m] (int numQPUs) {
      m.setNumQPUs(numQPUs);
      m.call();
    });

    // Sanity check
    INFO("Compare 2 blocks result with expected");
    INFO("num QPUs: " << m.numQPUs());
    compare_arrays(m.result(), expected, 1e-3f);   // just using same precision as full mult
  }


  std::cout << profile_output.dump();
  return (compiled != 0);
}


void test_simple_block(int dimension) {
    Float::Array2D a(dimension);
    Float::Array2D result(dimension);

    Matrix m(a, a);

    //
    // Check identity matrix
    //
    {
      a.make_unit_matrix();
      check_unitary(a);

      m.call();
      //std::cout << m.result().dump() << std::endl;
      INFO("Checking mult unit matrix");
      check_unitary(m.result());
    }

    //
    // Square of input matrix containing all ones
    //
    {
      a.fill(1);
      m.call();
      INFO("Checking mult all ones");
      compare_array_scalar(m.result(), (float) dimension);
    }

    //
    // Square of array with random values
    //
    {
      // Prepare input and expected result
      std::vector<float> expected;
      prepare_random(a, expected, dimension);

      m.call();
      INFO("Checking mult random values");

      float precision = -1.0f;
      if (dimension >  m.MAX_FULL_BLOCKS_V3D) {
        precision = 1e-3f;  // For big arrays using blocks
      }

      compare_arrays(m.result(), expected, precision);
    }
}

}  // anon namespace


TEST_CASE("Test block matrix multiplication [matrix][block]") {
  SUBCASE("Test simple block") {
    test_simple_block(2*16);
    test_simple_block(8*16);

    // Following works but takes long! Enable if you really want to check
    //test_simple_block(832);  // Test huge matrix above block limit as well
  }
}


TEST_CASE("Profile block matrix multiplication [matrix][block][profile]") {
  SUBCASE("Profile") {
    bool do_profiling = false;
    if (!do_profiling) return; 

    //LibSettings::heap_size(64 << 20);  // works! :-)

    // Profiling: try all sizes until compilation fails
    std::cout << "Block matrix compare" << ProfileOutput::header();

    int Step = 2;
    int N = 2; // 992 >> 4;
    bool can_continue = true;
    while (can_continue) {
      can_continue = profile_block_mult(16*N);
      N += Step;
      if (N > 20) break;
      //if (N > (992 >> 4)) break;
    }
  }
}


TEST_CASE("Test matrix mult on emulator [matrix][emu]") {
  Platform::use_main_memory(true);

  // NOTES:
  //   - Interpreter will not work, because VPM operations are not supported in it
  test_complex_matrix_multiplication(  2,  3*16,  2,  1, 1, {-1.0f, 2.0f}, { 1.0f, -1.0f }, EMULATE);
  test_complex_matrix_multiplication(  2,  4*16,  5,  1, 2, {-1.0f, 2.0f}, { 1.0f, -1.0f }, EMULATE);
  test_complex_matrix_multiplication(  3,  4*16,  17, 7, 1, {-1.0f, 2.0f}, { 1.0f, -1.0f }, EMULATE);

  Platform::use_main_memory(false);
}
