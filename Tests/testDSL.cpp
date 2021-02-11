#include "catch.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <V3DLib.h>
#include "LibSettings.h"
#include "support/support.h"

using namespace V3DLib;
using namespace std;

namespace {

//=============================================================================
// Helper methods
//=============================================================================

template<typename Array>
string showResult(Array &result, int index) {
  ostringstream buf;

  buf << "result  : ";
  for (int j = 0; j < 16; j++) {
    buf << result[16*index + j] << " ";
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


template<typename T>
void check_vector(SharedArray<T> &result, int index, std::vector<T> const &expected) {
  REQUIRE(expected.size() == 16);

  bool passed = true;
  int j = 0;
  for (; j < 16; ++j) {
    if (result[16*index + j] != expected[j]) {
      passed = false;
      break;
    }
  }

  INFO("j: " << j);
  INFO(showResult(result, index) << showExpected(expected));
  REQUIRE(passed);
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

void out(Int &res, Ptr<Int> &result) {
  *result = res;
  result = result + 16;
}


void test(Cond cond, Ptr<Int> &result) {
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
void test(BoolExpr cond, Ptr<Int> &result) {
  Int res = -1;  // temp variable for result of condition, -1 is unexpected value

  If (cond)
    res = 1;
  Else
    res = 0;
  End

  out(res, result);
}


void kernel_specific_instructions(Ptr<Int> result) {
  Int a = index();
  Int b = a ^ 1;
  out(b, result);
}


/**
 * @brief Kernel for testing If and When
 */
void kernelIfWhen(Ptr<Int> result) {
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


void check_conditionals(SharedArray<int> &result, int N) {
  vector<int> allZeroes = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  vector<int> allOnes   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

  auto assertResult = [N] ( SharedArray<int> &result, int index, std::vector<int> const &expected) {
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


class Complex;

namespace V3DLib {

struct ComplexExpr {
  // Abstract syntax tree
  Expr* expr;
  // Constructors
  ComplexExpr();
  //Complex(float x);
};

template <> inline Ptr<Complex> mkArg< Ptr<Complex> >() {
  Ptr<Complex> x;
  x = getUniformPtr<Complex>();
  return x;
}

template <> inline bool passParam< Ptr<Complex>, SharedArray<Complex>* >
  (Seq<int32_t>* uniforms, SharedArray<Complex>* p)
{
  uniforms->append(p->getAddress());
  return true;
}

}


class Complex {
public:
  enum {
    size = 2  // Size of instance in 32-bit values
  };

  Complex() {}

  Complex(const Complex &rhs) : Re(rhs.Re), Im(rhs.Im) {}

  Complex(PtrExpr<Float> input) {
    Re = *input;
    Im = *(input + 1);
  }

  Complex operator *(Complex rhs) {
    Complex tmp;
    tmp.Re = Re*rhs.Re - Im*rhs.Im;
    tmp.Im = Re*rhs.Im + Im*rhs.Re;

    return tmp;
  }

  Complex operator *=(Complex rhs) {
    Complex tmp;

    //FloatExpr tmpRe = Re*rhs.Re - Im*rhs.Im;
    tmp.Re = Re*rhs.Re - Im*rhs.Im;
    tmp.Im = Re*rhs.Im + Im*rhs.Re;

    return tmp;
  }

  Float Re;
  Float Im;
};


void kernelComplex(Ptr<Float> input, Ptr<Float> result) {
  auto inp = input + 2*index();
  auto out = result + 2*index();

  //Complex a(input + 2*index());
  Complex a;
  a.Re = *inp;
  a.Im = *(inp + 1);
  Complex b = a*a;
  *out = b.Re;
  *(out + 1) = b.Im;
}


//=============================================================================
// Unit tests
//=============================================================================

TEST_CASE("Test correct working DSL", "[dsl]") {
  const int N = 25;  // Number of expected result vectors

  SECTION("Test specific instructions") {
    int const NUM = 1;
    vector<int> expected = {1,0,3,2,5,4,7,6,9,8,11,10,13,12,15,14};

    auto k = compile(kernel_specific_instructions);
    //k.pretty(true);

    SharedArray<int> result(16*NUM);

    result.fill(-2);  // Initialize to unexpected value
     k.load(&result).emu();
    check_vector(result, 0, expected);

    result.fill(-2);
     k.load(&result).interpret();
    check_vector(result, 0, expected);

    result.fill(-2);
     k.load(&result).call();
    check_vector(result, 0, expected);
  }


  //
  // Test all variations of If and When
  //
  SECTION("Conditionals work as expected") {
    // Construct kernel
    auto k = compile(kernelIfWhen);

    SharedArray<int> result(16*N);

    // Reset result array to unexpected values
    auto reset = [&result] () {
      result.fill(-2);  // Initialize to unexpected value
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


TEST_CASE("Test construction of composed types in DSL", "[dsl]") {

  SECTION("Test Complex composed type") {
    // TODO: No assertion in this part, need any?

    const int N = 1;  // Number Complex items in vectors

    // Construct kernel
    auto k = compile(kernelComplex);

    // Allocate and array for input and result values
    SharedArray<float> input(2*16*N);
    input[ 0] = 1; input[ 1] = 0;
    input[ 2] = 0; input[ 3] = 1;
    input[ 3] = 1; input[ 4] = 1;

    SharedArray<float> result(2*16*N);

    // Run kernel
    k.load(&input, &result).call();

    //cout << showResult(result, 0) << endl;
  }
}


//-----------------------------------------------------------------------------
// Test for specific DSL operations.
//-----------------------------------------------------------------------------

void int_ops_kernel(Ptr<Int> result) {
  using namespace V3DLib::functions;

  auto store = [&result] (IntExpr const &val) {
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

  store(16*16/index());
  store((-16*16)/(-index()));
  store((-16*16)/index());
  store(16*16/(-index()));
}


void float_ops_kernel(Ptr<Float> result) {
  Float a = toFloat(index());
  a += 3.0f;
  a += 0.25f;

  *result = a;
}


TEST_CASE("Test specific operations in DSL", "[dsl][ops]") {
  SECTION("Test integer operations") {
    int const N = 8;  // Number of expected results

    auto k = compile(int_ops_kernel);
    //k.pretty(true);

    SharedArray<int> result(16*N);

    k.load(&result);
    //k.interpret();
    //k.emu();
    k.call();
    //dump_array(result, 16);

    vector<vector<int>> expected = {
      {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},                    // +=
      {-8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7},                     // -=
      {8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7},                             // abs
      {8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7},                      // 2-s complement

      // integer division
      {2147483647, 256, 128, 85, 64, 51, 42, 36, 32, 28, 25, 23, 21, 19, 18, 17},   // First value is 'infinity'
      {-2147483647, 256, 128, 85, 64, 51, 42, 36, 32, 28, 25, 23, 21, 19, 18, 17},  // NB -0 == 0
      {-2147483647, -256, -128, -85, -64, -51, -42, -36, -32, -28, -25, -23, -21, -19, -18, -17},
      {2147483647, -256, -128, -85, -64, -51, -42, -36, -32, -28, -25, -23, -21, -19, -18, -17},
    };

    check_vectors(result, expected);
  }


  SECTION("Test float operations") {
    int const N = 1;  // Number of expected results

    auto k = compile(float_ops_kernel);

    SharedArray<float> result(16*N);

    k.load(&result).call();

    vector<float> expected = { 3.25,  4.25,  5.25,  6.25,  7.25,  8.25,  9.25, 10.25,
                              11.25, 12.25, 13.25, 14.25, 15.25, 16.25, 17.25, 18.25};
    //dump_array(result);
    check_vector(result, 0, expected);
  }
}


void nested_for_kernel(Ptr<Int> result) {
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


TEST_CASE("Test For-loops", "[dsl][for]") {
  Platform::use_main_memory(true);

  SECTION("Test nested For-loops") {
    auto k = compile(nested_for_kernel);

    SharedArray<int> result(16);
    k.load(&result).emu();
    //dump_array(result);

    vector<int> expected = {18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27};
    check_vector(result, 0, expected);
  }

  Platform::use_main_memory(false);
} 


template<typename T>
void rot_kernel(Ptr<T> result, Ptr<T> a) {
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
TEST_CASE("Test rotate on emulator", "[emu][rotate]") {
  Platform::use_main_memory(true);
  int const N = 4;

  SharedArray<int> a(16);
  SharedArray<int> result1(N*16);
  result1.fill(-1);
  SharedArray<int> result2(N*16);
  result2.fill(-1);

  auto reset = [&a] () {
    for (int i = 0; i < (int) a.size(); i++) {
      a[i] = (i + 1);
      //a[i] = (float) (i + 1);
    }
  };

  auto k = compile(rot_kernel<Int>);
  k.pretty(true, "obj/test/rot_kernel.txt", false);
  //k.load(&result1, &a);

  // Interpreter works fine, used here to compare emulator output
  reset();
  k.interpret();
  //dump_array(a);
  //dump_array(result1, 16);

  std::cout << "\n";

  reset();
  k.load(&result2, &a);
  k.emu();
  //dump_array(a);
  //dump_array(result2, 16);

  REQUIRE(result1 == result2);

  Platform::use_main_memory(false);
}


/**
 * This should try out all the possible ways of reading and writing
 * main memory.
 */
template<typename T>
void offsets_kernel(Ptr<T> result, Ptr<T> src) {
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
TEST_CASE("Initialization with index() on uniform pointers should work as expected", "[dsl][offsets]") {
  int const N = 6;

  SharedArray<int> a(3*16);
  SharedArray<int> result(N*16);

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


  SECTION("Test with TMU") {
    auto k = compile(offsets_kernel<Int>);
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


  SECTION("Test with DMA") {
    LibSettings::use_tmu_for_load(false);

    auto k = compile(offsets_kernel<Int>);
    //k.pretty(true, nullptr, false);
    k.load(&result, &a);

    reset();
    k.interpret();
    check("dma interpreter");

    reset();
    k.emu();
    check("dma emulator");

    reset();
    k.call();
    //dump_array(result, 16);
    check("dma qpu");

    LibSettings::use_tmu_for_load(false);
  }
}
