#include <iostream>
#include <string>
#include <sstream>
#include <V3DLib.h>
#include "LibSettings.h"
#include "Support/pgm.h"
#include "support/support.h"
#include "Source/Complex.h"
#include "Source/Functions.h"

using namespace V3DLib;
using namespace std;

namespace {

//=============================================================================
// Helper methods
//=============================================================================

/**
 * Generate cos values to compare with
 */
std::vector<float> lib_cos_values(int size, float freq = -1.0f, float offset = 0.0f) {
  if (freq == -1.0f) {
    freq = 1.0f/((float) size);
  }

  std::vector<float> ret;
  ret.resize(size);

  for (int x = 0; x < size; ++x) {
    ret[x] = cos((float) (freq*(2*M_PI)*(((float) x) - offset)));
  }

  return ret;
}


std::vector<float> lib_sin_values(int size, float freq = -1.0f, float offset = 0.0f) {
  if (freq == -1.0f) {
    freq = 1.0f/((float) size);
  }

  std::vector<float> ret;
  ret.resize(size);

  for (int x = 0; x < size; ++x) {
    ret[x] = sin((float) (freq*(2*M_PI)*(((float) x) - offset)));
  }

  return ret;
}


/**
 * Calculate max abs difference for arrays
 */
float max_abs_value(std::vector<float> const &a, float const *b) {
  REQUIRE(b != nullptr);

  float ret = -1.0f;

  for (int i = 0; i < (int) a.size(); ++i) {
    float diff = abs(a[i] - b[i]);
    if (ret == -1.0f || ret < diff) {
      ret = diff;
    }
  }

  return ret;
} 


template<typename Array>
string showResult(Array &result, int index, int size = 16) {
  REQUIRE(size % 16 == 0);
  ostringstream buf;

  buf << "result  : ";
  for (int j = 0; j < size; j++) {
    buf << result[size*index + j] << " ";
  }
  buf << "\n";

  return buf.str();
}


template<typename T>
string showExpected(const std::vector<T> &expected) {
  ostringstream buf;

  buf << "expected: ";
  for (int j = 0; j < 16; j++) {
    buf << expected[j] << " ";
  }
  buf << "\n";

  return buf.str();
}


/**
 * Compare 16-value block in `result` starting at `index` with expected value.
 */
template<typename T>
void check_vector(SharedArray<T> &result, int index, std::vector<T> const &expected, float precision = 0.0f) {
  REQUIRE(expected.size() == 16);

  bool passed = true;
  int j = 0;
  for (; j < 16; ++j) {
    if (abs((float)result[16*index + j] - (float) expected[j]) > precision) {
      passed = false;
      break;
    }
  }

  INFO("index: " << index << ", j: " << j);
  INFO(showResult(result, index) << showExpected(expected));
  REQUIRE(passed);
}


/**
 * Overload which assumes that all elements of the 16-value block have the same values
 */
template<typename T>
void check_vector(SharedArray<T> &result, int index, int expected, float precision = 0.0f) {
  std::vector<T> vec;
  vec.resize(16);

  for (int i = 0; i < (int) vec.size(); ++i) {
    vec[i] = expected;
  }

  check_vector(result, index, vec, precision);
}


template<typename T>
void check_vectors(SharedArray<T> &result, std::vector<std::vector<T>> const &expected) {
  for (int index = 0; index < (int) expected.size(); ++index) {
    check_vector(result, index, expected[index]);
  }
}

}  // namespace


//=============================================================================
// Kernel definition(s)
//=============================================================================

void out(Int &res, Int::Ptr &result) {
  *result = res;
  result = result + 16;
}


void out(Float &res, Float::Ptr &result) {
  *result = res;
  result = result + 16;
}


void test(Cond cond, Int::Ptr &result) {
  Int res = -1;  // temp variable for result of condition, -1 is unexpected value

  If (cond)
    res = 1;
  Else
    res = 0;
  End

  out(res, result);
}


/**
 * @brief Overload for BoolExpr
 *
 * TODO: Why is distinction BoolExpr <-> Cond necessary? Almost the same
 */
void test(BoolExpr cond, Int::Ptr &result) {
  Int res = -1;  // temp variable for result of condition, -1 is unexpected value

  If (cond)
    res = 1;
  Else
    res = 0;
  End

  out(res, result);
}


void kernel_specific_instructions(Int::Ptr result) {
  Int a = index();
  Int b = a ^ 1;
  out(b, result);
}


void kernel_specific_float_instructions(Float::Ptr result) {
  Float a = toFloat(index() + 1);
  //Float b = a / b; - seq fault! TODO detect
  Float b = 1 / a;
  out(b, result);
}



/**
 * @brief Kernel for testing If and When
 */
void kernelIfWhen(Int::Ptr result) {
  Int outIndex = index();
  Int a = index();

  // any
  test(any(a <   0), result);
  test(any(a <   8), result);
  test(any(a <=  0), result);  // Boundary check
  test(any(a >= 15), result);  // Boundary check
  test(any(a <  32), result);
  test(any(a >  32), result);

  // all
  test(all(a <   0), result);
  test(all(a <   8), result);
  test(all(a <=  0), result);  // Boundary check
  test(all(a >= 15), result);  // Boundary check
  test(all(a <  32), result);
  test(all(a >  32), result);

  // Just If - should be same as any
  test((a <   0), result);
  test((a <   8), result);
  test((a <=  0), result);     // Boundary check
  test((a >= 15), result);     // Boundary check
  test((a <  32), result);
  test((a >  32), result);

  // When
  Int res = -1;  // temp variable for result of condition, -1 is unexpected value
  Where (a < 0) res = 1; Else res = 0; End
  out(res, result);

  res = -1;
  Where (a <= 0) res = 1; Else res = 0; End  // Boundary check
  out(res, result);

  res = -1;
  Where (a >= 15) res = 1; Else res = 0; End  // Boundary check
  out(res, result);

  res = -1;
  Where (a < 8) res = 1; Else res = 0; End
  out(res, result);

  res = -1;
  Where (a >= 8) res = 1; Else res = 0; End
  out(res, result);

  res = -1;
  Where (a < 32) res = 1; Else res = 0; End
  out(res, result);

  res = -1;
  Where (a > 32) res = 1; Else res = 0; End
  out(res, result);
}


void check_conditionals(Int::Array &result, int N) {
  vector<int> allZeroes = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  vector<int> allOnes   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

  auto assertResult = [N] ( Int::Array &result, int index, std::vector<int> const &expected) {
    INFO("index: " << index);
    REQUIRE(result.size() == (unsigned) N*16);
    check_vector(result, index, expected);
  };

  // any
  assertResult(result,  0, allZeroes);
  assertResult(result,  1, allOnes);
  assertResult(result,  2, allOnes);
  assertResult(result,  3, allOnes);
  assertResult(result,  4, allOnes);
  assertResult(result,  5, allZeroes);
  // all
  assertResult(result,  6, allZeroes);
  assertResult(result,  7, allZeroes);
  assertResult(result,  8, allZeroes);
  assertResult(result,  9, allZeroes);
  assertResult(result, 10, allOnes);
  assertResult(result, 11, allZeroes);
  // Just If - should be same as any
  assertResult(result, 12, allZeroes);
  assertResult(result, 13, allOnes);
  assertResult(result, 14, allOnes);
  assertResult(result, 15, allOnes);
  assertResult(result, 16, allOnes);
  assertResult(result, 17, allZeroes);
  // where
  assertResult(result, 18, allZeroes);
  assertResult(result, 19, {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});
  assertResult(result, 20, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1});
  assertResult(result, 21, {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0});
  assertResult(result, 22, {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1});
  assertResult(result, 23, allOnes);
  assertResult(result, 24, allZeroes);
}


void complex_kernel(Complex::Ptr input, Complex::Ptr result) {
  Complex a = *input;
  Complex b = a*a;
  *result = b;
}


//=============================================================================
// Unit tests
//=============================================================================

TEST_CASE("Test correct working DSL [dsl]") {
  const int N = 25;  // Number of expected result vectors

  SUBCASE("Test specific int instructions") {
    int const NUM = 1;
    vector<int> expected = {1,0,3,2,5,4,7,6,9,8,11,10,13,12,15,14};

    Int::Array result(16*NUM);
    result.fill(-2);  // Initialize to unexpected value

    auto k = compile(kernel_specific_instructions);
    k.load(&result).interpret();
    check_vector(result, 0, expected);

    result.fill(-2);
    k.load(&result).emu();
    check_vector(result, 0, expected);

    result.fill(-2);
    k.load(&result).call();
    check_vector(result, 0, expected);
  }


  SUBCASE("Test specific float instructions") {
    vector<float> expected;
   	expected.resize(16); 

    for (int i = 0; i < 16; ++i) {
      expected[i] = 1.0f/(1.0f + ((float) i));
    }

    Float::Array result(16);
    result.fill(-2);  // Initialize to unexpected value

    auto k = compile(kernel_specific_float_instructions);
    k.load(&result);
    k.interpret();
    check_vector(result, 0, expected);

    result.fill(-2);
    k.load(&result).emu();
    check_vector(result, 0, expected);

    result.fill(-2);
    k.load(&result).call();

    float precision = 0.0f;
    if (Platform::has_vc4()) {
      precision = 0.5e-4f;
    }

    check_vector(result, 0, expected, precision);

  }


  //
  // Test all variations of If and When
  //
  SUBCASE("Conditionals work as expected") {
    auto k = compile(kernelIfWhen);

    Int::Array result(16*N);

    // Reset result array to unexpected values
    auto reset = [&result] () {
      result.fill(-2);
    };

    //
    // Run kernel in the three different run modes
    //
    reset();
    k.load(&result).call();
    check_conditionals(result, N);

    reset();
    k.load(&result).emu();
    check_conditionals(result, N);

    reset();
    k.load(&result).interpret();
    check_conditionals(result, N);
  }
}


TEST_CASE("Test construction of composed types in DSL [dsl][complex]") {
  SUBCASE("Test Complex composed type") {
    const int N = 1;  // Number Complex items in vectors

    auto k = compile(complex_kernel);

    // Allocate and array for input and result values
    Complex::Array input(16*N);
    input.fill({0,0});
    input[0] = { 1, 0};
    input[1] = { 0, 1};
    input[2] = { 1, 1};

    Complex::Array result(16*N);

    k.load(&input, &result).call();

/*
    std::cout << input.dump();
    std::cout << result.dump();

    std::cout << result[0].dump()     << "\n";
    std::cout << complex(1, 0).dump();
    std::cout << std::endl;
*/

    REQUIRE(result[0] ==  complex(1, 0));
    REQUIRE(result[1] ==  complex(-1, 0));
    REQUIRE(result[2] ==  complex(0, 2));
  }
}


//-----------------------------------------------------------------------------
// Test for specific DSL operations.
//-----------------------------------------------------------------------------

void int_ops_kernel(Int::Ptr result) {
  using namespace V3DLib::functions;

  //
  // NEVER FORGET:
  //
  // Previous definition:
  //
  //    auto store = [&result] (IntExpr const &val) {
  //
  // This resulted in the passed Int var to be converted to IntExpr,
  // and then back to Int, creating a useless interim variable in the source lang
  //
  auto store = [&result] (Int const &val) {
    comment("store starts next"); 
    *result = val;
    result += 16;
  };

  Int a = index();
  a += 3;
  store(a);

  a -= 11;
  store(a);

  store(abs(index() - 8));
  store(two_complement(index() - 8));       // 2's complement, library call

  Int b = topmost_bit(1 << (index() + 3));
  store(b);

  b = -256;
  store(b);

  comment("First division test starts next");
  store(16*16/index());

  comment("First usage -index() starts next");
  store((-16*16)/(-index()));

  store((-16*16)/index());
  store(16*16/(-index()));
  store(17*index()/11);
}


void float_ops_kernel(Float::Ptr result) {
  Float a = toFloat(index());
  a += 3.0f;
  a += 0.25f;

  *result = a;
}


TEST_CASE("Test specific operations in DSL [dsl][ops]") {
  SUBCASE("Test integer operations") {
    int const N = 11;  // Number of expected results

    auto k = compile(int_ops_kernel);

    Int::Array result(16*N);
    result.fill(-1);

    k.load(&result);
/*
    k.pretty(true, "obj/test/int_ops_kernel_vc4.txt", true);
    k.dump_compile_data(false, "obj/test/int_ops_kernel_compile_data_v3d.txt");
    k.pretty(false, "obj/test/int_ops_kernel_v3d.txt", true);
*/
    k.call();

    vector<vector<int>> expected = {
      {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},                    // +=
      {-8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7},                     // -=
      {8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7},                             // abs
      {8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7},                      // 2-s complement
      {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},                    // topmost_bi 
      {-256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256}, // b = -256 
      // integer division
      {2147483647, 256, 128, 85, 64, 51, 42, 36, 32, 28, 25, 23, 21, 19, 18, 17},   // First value is 'infinity'
      {-2147483647, 256, 128, 85, 64, 51, 42, 36, 32, 28, 25, 23, 21, 19, 18, 17},  // NB -0 == 0
      {-2147483647, -256, -128, -85, -64, -51, -42, -36, -32, -28, -25, -23, -21, -19, -18, -17},
      {2147483647, -256, -128, -85, -64, -51, -42, -36, -32, -28, -25, -23, -21, -19, -18, -17},
      {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 17, 18, 20, 21, 23}
    };

    check_vectors(result, expected);
  }


  SUBCASE("Test float operations") {
    int const N = 1;  // Number of expected results

    auto k = compile(float_ops_kernel);

    Float::Array result(16*N);

    k.load(&result).call();

    vector<float> expected = { 3.25,  4.25,  5.25,  6.25,  7.25,  8.25,  9.25, 10.25,
                              11.25, 12.25, 13.25, 14.25, 15.25, 16.25, 17.25, 18.25};
    check_vector(result, 0, expected);
  }
}


void nested_for_kernel(Int::Ptr result) {
  int const COUNT = 3;
  Int x = 0;

  For (Int n = 0, n < COUNT, n++)
    For (Int m = 0, m < COUNT, m++)
      x += 1;

      Where ((index() & 0x1) == 1)
        x += 1;
      End

      If ((m & 0x1) == 1)
        x += 1;
      End
    End

    x += 2;
  End

  *result = x;
}


TEST_CASE("Test For-loops [dsl][for]") {
  Platform::use_main_memory(true);

  SUBCASE("Test nested For-loops") {
    auto k = compile(nested_for_kernel);

    Int::Array result(16);
    k.load(&result).emu();

    vector<int> expected = {18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27};
    check_vector(result, 0, expected);
  }

  Platform::use_main_memory(false);
} 


template<typename T, typename Ptr>
void rot_kernel(Ptr result, Ptr a) {
  T val = *a;
  T val2 = *a;

  val2 = rotate(val, 1);
  *result = val2; result.inc();

  val2 += rotate(val, 1);
  *result = val2; result.inc();

  rotate_sum(val, val2);
  *result = val2; result.inc();

  T val3 = val;
  set_at(val3, 0, val2);
  *result = val3;
}


// This went wrong at some point
TEST_CASE("Test rotate on emulator [emu][rotate]") {
  Platform::use_main_memory(true);
  int const N = 4;

  Int::Array a(16);
  Int::Array result1(N*16);
  result1.fill(-1);
  Int::Array result2(N*16);
  result2.fill(-1);

  auto reset = [&a] () {
    for (int i = 0; i < (int) a.size(); i++) {
      a[i] = (i + 1);
      //a[i] = (float) (i + 1);
    }
  };

  auto k = compile(rot_kernel<Int, Int::Ptr>);
  k.load(&result1, &a);

  // Interpreter works fine, used here to compare emulator output
  reset();
  k.interpret();

  std::cout << "\n";

  reset();
  k.load(&result2, &a);
  k.emu();

  REQUIRE(result1 == result2);

  Platform::use_main_memory(false);
}


/**
 * This should try out all the possible ways of reading and writing
 * main memory.
 */
template<typename T, typename Ptr>
void offsets_kernel(Ptr result, Ptr src) {
  Int a = index();
  *result = a;
  result.inc();

  T val = *src;
  *result = val;

  T val2 = *(src + 32);
  *(result + 16) = val2;

  val2 = src[32];
  result[32] = val2;

  src.inc();
  result.inc();
  result.inc();
  result.inc();

  val = *src;
  *result = val;
  result.inc();

  gather(src);  comment("Start gather test");
  receive(a);
  *result = a;
}


/**
 * Created in order to test init uniforms pointers with index() for vc4
 */
TEST_CASE("Initialization with index() on uniform pointers should work as expected [dsl][offsets]") {
  int const N = 6;

  Int::Array a(3*16);
  Int::Array result(N*16);

  std::vector<int> expected = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 
     1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};

  REQUIRE(expected.size() == N*16);
  REQUIRE(result.size() == expected.size());

  auto reset = [&a, &result] () {
    // Not necessary at this point, a does not change
    for (int i = 0; i < (int) a.size(); i++) {
      a[i] = (i + 1);
    }

    result.fill(-1);
  };

  auto check = [&result, &expected] (char const *label) {
    for (int i = 0; i < (int) result.size(); i++) {
      INFO("label: " << label << ", row: " << (i/16) << ", index: " << (i %16));
      REQUIRE(result[i] == expected[i]);
    }
  };


  SUBCASE("Test with TMU") {
    auto k = compile(offsets_kernel<Int, Int::Ptr>);
    k.load(&result, &a);

    reset();
    k.interpret();
    check("tmu interpeter");

    reset();
    k.emu();
    check("tmu emulator");

    reset();
    k.call();
    check("tmu qpu");
  }


  SUBCASE("Test with DMA") {
    LibSettings::use_tmu_for_load(false);

    auto k = compile(offsets_kernel<Int, Int::Ptr>);
    k.load(&result, &a);

    reset();
    k.interpret();
    check("dma interpreter");

    reset();
    k.emu();
    check("dma emulator");

    reset();
    k.call();
    check("dma qpu");

    LibSettings::use_tmu_for_load(false);
  }
}


void cosine_kernel(Float::Ptr result, Int numValues, Float freq, Int offset) {
  For (Int n = 0, n < numValues, n += 16)
    Float x = freq*toFloat(n + index() - offset);
    *result = functions::cos(x);
    result.inc();
  End
}


void sine_kernel(Float::Ptr result, Int numValues, Float freq, Int offset) {
  For (Int n = 0, n < numValues, n += 16)
    Float x = freq*toFloat(n + index() - offset);
    *result = functions::sin(x);
    result.inc();
  End
}


void floor_kernel(Float::Ptr result, Float::Ptr input, Int numValues) {
  For (Int n = 0, n < numValues, n += 16)
    *result = functions::ffloor(*input);
    result.inc(); input.inc();
  End
}


void fabs_kernel(Float::Ptr result, Float::Ptr input, Int numValues) {
  For (Int n = 0, n < numValues, n += 16)
    *result = functions::fabs(*input);
    result.inc(); input.inc();
  End
}


template< typename T1, typename T2>
float calc_max_diff(T1 &arr1, T2 &arr2, int size) { 
  float max_diff = 0.0f;

  for (int x = 0; x < size; ++x) {
    float tmp = std::abs(arr1[x] - arr2[x]);
    if (tmp > max_diff) max_diff = tmp;
  }

  return max_diff;
}


TEST_CASE("Test functions [dsl][func]") {
  int const NumValues       = 15;
  int const SharedArraySize = (NumValues/16 +1)*16;

  float input[NumValues];

  int n = 0;
  input[n] =  1.0f; n++;
  input[n] =  1.3f; n++;
  input[n] = -1.0f; n++;
  input[n] = -1.3f; n++;
  input[n] =  0.9f; n++;
  input[n] = -0.9f; n++;
  input[n] =  1.0e-32f; n++;
  input[n] = -1.0e-32f; n++;
  input[n] =  1.1e38f; n++;
  input[n] = -1.1e38f; n++;
  //input[n] = -1.1e-38f; n++;  // On v3d, this works as expected. On vc4, this registers as 0.0, not negative
  input[n] = -1.1e-36f; n++;    // Using this value instead
  input[n] =  7.0f; n++;
  input[n] =  7.1f; n++;
  input[n] = -7.0f; n++;
  input[n] = -7.1f; n++;


  Float::Array input_qpu(SharedArraySize);
  for (int n = 0; n < NumValues; ++n) {
   input_qpu[n] = input[n];
  }


  /**
   * NOTE: Remember, sin/cos normalized on 2*M_PI
   */
  SUBCASE("Test trigonometric functions") {
    float const MAX_DIFF = 0.57f;  // Test value for extra_precision == false

    const int size   = 1000;
    const int offset = size/2;
    const float freq = (float) (1.0f/((double) size));

    auto lib_cos = lib_cos_values(size, freq, offset);  // cos lib values, to compare with

    //
    // Calc with scalar kernel
    //
    float scalar_cos[size];

    {
      for (int x = 0; x < size; ++x) {
        scalar_cos[x] = functions::cos(freq*((float) (x - offset)));
      };

      float max_diff = calc_max_diff(scalar_cos, lib_cos, size); 
      INFO("Max diff: " << max_diff);
      REQUIRE(max_diff < MAX_DIFF);
    }

    //
    // Calc with QPU kernel
    //
    Float::Array qpu_cos(size);
    Float::Array qpu_sin(size);

    {
      auto k = compile(cosine_kernel);
      //k.pretty(false, nullptr, true);
      k.load(&qpu_cos, size, freq, offset);
      k.call();

      float max_diff = calc_max_diff(lib_cos, qpu_cos, size); 
      //printf("Max diff: %f\n", max_diff);
      INFO("Max diff: " << max_diff);
      REQUIRE(max_diff < MAX_DIFF);
    }

    {
      auto k = compile(sine_kernel);
      k.load(&qpu_sin, size, freq, offset);
      k.call();
    }

    PGM pgm(size, 400);
    pgm.plot(lib_cos, 64)
     //.plot(scalar_cos, size)
       .plot(qpu_cos.ptr(), size, 32)
       .plot(qpu_sin.ptr(), size, 32)
       .save("obj/test/cos_plot.pgm");
  }


  SUBCASE("Test ffloor()") {
    float results_scalar[NumValues];
    for (int n = 0; n < NumValues; ++n) {
     results_scalar[n] = (float) floor(input[n]);
    }

    Float::Array results_qpu(SharedArraySize);
    results_qpu.fill(-1.0f);

    auto k = compile(floor_kernel);
    //k.pretty(false, nullptr, true);
    k.load(&results_qpu, &input_qpu, NumValues);
    k.call();

    for (int n = 0; n < NumValues; ++n) {
      INFO("input_qpu     : " << dump_array2(input_qpu));
      INFO("results_scalar: " << dump_array2(results_scalar, NumValues));
      INFO("results_qpu   : " << dump_array2(results_qpu));
      INFO("n: " << n);
      REQUIRE(results_scalar[n] == results_qpu[n]);
    }
  }


  SUBCASE("Test fabs()") {
    float results_scalar[NumValues];
    for (int n = 0; n < NumValues; ++n) {
     results_scalar[n] = (float) abs(input[n]);
    }

    Float::Array results_qpu(SharedArraySize);
    results_qpu.fill(-1.0f);

    auto k = compile(fabs_kernel);
    k.load(&results_qpu, &input_qpu, NumValues);
    k.call();

    for (int n = 0; n < NumValues; ++n) {
      INFO("results_scalar: " << dump_array2(results_scalar, NumValues));
      INFO("results_qpu   : " << dump_array2(results_qpu));
      INFO("n: " << n);
      REQUIRE(results_scalar[n] == results_qpu[n]);
    }
  }
}


//=============================================================================
// Test Issues
//
// Test stuff which has been seen to go wrong.
//=============================================================================

namespace {

void issues_kernel(Int::Ptr result, Int::Ptr src) {
  Int a = 0;       comment("Start check 'If (a != b)' same as 'If (any(a !=b))'");
  Int c = 0;

  For (Int b = 0, b < 2, b++)
    // Generation of this and following If should be identical - visual check
    If (a != b)
      c = 1;
    End

    *result = c; result.inc();

    c = 0;

    If (any(a != b))
     c = 1;
    End

    *result = c; result.inc();
  End

  Int dummy = 0;   comment("Start ptr offset check");
  *result = 4*(index() + 16*me());
  result.inc();

  *result = *src;  comment("Check *dst = *src"); 
}


//
// Following should all generate errors during compile
//
void init_self_1_kernel() { Int x = x; }
void init_self_2_kernel() { Float x = x; }
void init_self_3_kernel() { Complex y = y; }

}  // anon namespace


TEST_CASE("Test issues [dsl][issues]") {
  Platform::use_main_memory(true);

  SUBCASE("Verify issues") {
    int const N = 6;

    auto k = compile(issues_kernel);
    //k.pretty(true, "obj/test/issues_kernel_vc4.txt", false);
    //k.pretty(false, "obj/test/issues_kernel_v3d.txt");

    Int::Array input(16);
    input.fill(7);

    Int::Array result(16*N);
    k.load(&result, &input);
    k.emu();

    check_vector(result, 0, 0);
    check_vector(result, 1, 0);
    check_vector(result, 2, 1);
    check_vector(result, 3, 1);

    std::vector<int> expected = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60};
    check_vector(result, 4, expected);
    check_vector(result, 5, 7);
    //std::cout << showResult(result, 5) << std::endl;
  }


  /**
   * The issue here is that initialization like:
   *
   *   Int x = x + 1;
   *
   * ... is allowed by C++ syntax. There is no way to prevent this, other
   * than hoping that the compiler flags this as warning. In the given example,
   * no warning is given.
   *
   * The only good way to deal with this, is to just be aware of it.
   *
   * This test only checks for `Int x = x;`, the simplest case possible.
   * Anything more elaborate, forget it. I've racked my brain on this, there is no salvation.
   */
  SUBCASE("Check init self issue") {
    {
      auto k = compile(init_self_1_kernel);
      REQUIRE(k.has_errors());
    }

    {
      auto k = compile(init_self_2_kernel);
      REQUIRE(k.has_errors());
    }

    {
      auto k = compile(init_self_3_kernel);
      REQUIRE(k.has_errors());
    }
  }

  Platform::use_main_memory(false);
}


void sincos_kernel(Float::Ptr result, Int size) {
  Int count = size >> 4;

  For (Int n = 0, n < count, n++)
    Float param = toFloat((n << 4) + index())/toFloat(size);

    Float val  = functions::sin(param, true);
    *result = val;  result.inc();
  End

  For (Int n = 0, n < count, n++)
    Float param = toFloat((n << 4) + index())/toFloat(size);

    Float val  = functions::sin(param, false);
    *result = val;  result.inc();
  End

  For (Int n = 0, n < count, n++)
    Float param = toFloat((n << 4) + index())/toFloat(size);

    Float instr_val = sin(param);
    *result = instr_val;  result.inc();
  End

  For (Int n = 0, n < count, n++)
    Float param = toFloat((n << 4) + index())/toFloat(size);

    Float instr_val = sin(param*-1);
    *result = instr_val;  result.inc();
  End

  For (Int n = 0, n < count, n++)
    Float param = toFloat((n << 4) + index())/toFloat(size);

    Float instr_val = cos(param);       // This one not unit tested
    *result = instr_val;  result.inc();
  End
}


TEST_CASE("Test sin/cos instructions [dsl][sincos]") {
  int const N = 5*16;

  Float::Array result(5*N);
  auto lib_sin = lib_sin_values(N);  // cos lib values, to compare with

  auto k = compile(sincos_kernel);
  //k.pretty(false);
  k.load(&result, N);
  k.call();

  float const hi_precision = 1.1e-3f;
  float const lo_precision = 5.7e-2f;
  float const v3d_precision = (Platform::compiling_for_vc4())?lo_precision:1.0e-6f;  // vc4 will use the lo-res sin function, v3d the hardware, which is really precise

  {
    float diff = max_abs_value(lib_sin, result.ptr());
    INFO("max abs diff hi-prec sin: " << diff);
    REQUIRE(diff <= hi_precision);
  }

  {
    float diff = max_abs_value(lib_sin, result.ptr() + N);
    INFO("max abs diff lo-prec sin: " << diff);
    REQUIRE(diff <= lo_precision);
  }

  {
    float diff = max_abs_value(lib_sin, result.ptr() + 2*N);
    INFO("max abs diff v3d sin: " << diff);
    REQUIRE(diff <= v3d_precision);
  }

//  debug(showResult(lib_sin, 0, N));
//  debug(showResult(result, 4, N));

  // Check proper values negative sin
  // You would expect this to be exact, but tiny differences crop up.
  float const neg_precision = 1.0e-6f;
  for (int i = 0; i < N; ++i) {
    REQUIRE(abs(result[2*N + i] + result[3*N + i]) < neg_precision);
  }
}
