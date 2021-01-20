///////////////////////////////////////////////////////////////////////////////
//
// This tests the required components for matrix multiplication
//
///////////////////////////////////////////////////////////////////////////////
#include "catch.hpp"
#include <string>
#include <V3DLib.h>
#include "Support/basics.h"

namespace {
using namespace V3DLib;

void dump(float *a, int size,  int linesize = -1) {
  std::string str("<");

  for (int i = 0; i < (int) size; i++) {

    if (linesize != -1) {
      if (i % linesize == 0) {
        str << "\n";
      }
    }
    str << a[i] << ", " ;
  }

  str << ">";
  printf("%s\n", str.c_str());
};


void dump(SharedArray<float> &a, int linesize = -1) {
  std::string str("<");

  for (int i = 0; i < (int) a.size(); i++) {

    if (linesize != -1) {
      if (i % linesize == 0) {
        str << "\n";
      }
    }
    str << a[i] << ", " ;
  }

  str << ">";
  printf("%s\n", str.c_str());
};


/**
 * CPU version of matrix multiplication, naive implementation
 *
 * Matrixes are assumed to be square.
 *
 * @param N  dimension of square matrices
 * @param c  Pointer to result array
 */
void matrix_mult_scalar(int N, float *c, float *a, float *b) {
  for (int x = 0; x < N; x++) {
    for (int y = 0; y < N; y++) {
      float result = 0;

      for (int i = 0; i < N; i++) {
        result += a[i + y*N] * b [x + i*N];
      }

      c[x + y*N] = result;
    }
  }
}


/**
 * Sum up all the vector elements of a register.
 *
 * All vector elements of register result will contain the same value.
 *
 * This is a kernel helper function.
 */
void rotate_sum(Float &input, Float &result) {
  result = input;

  Float tmp = input;
  for (int i = 0; i < 15; i++) {
    tmp = rotate(tmp, 1);
    //result += tmp;  // TODO this should work
    result = result + tmp;
  }
}


void check_sum(Ptr<Float> input, Ptr<Float> result) {
  Float val = *input;
  Float sum;

  rotate_sum(val, sum);

  *result = sum;
}


/**
 * Set value of src to vector element 'n' of dst
 *
 * All other values in dst are untouched.
 *
 * This is a kernel helper function.
 *
 * @param n  index of vector element to set. Must be in range 0..15 inclusive
 */
void set_at(Float &dst, Int n, Float &src) {
  Where(index() == n)
    dst = src;
  End 
}


void check_set_at(Ptr<Float> input, Ptr<Float> result, Int index) {
  Float a = *input;
  Float b = *result;

  set_at(b, index, a);

  *result = b;
}

/**
 * This is a kernel helper class
 */
class DotVector {
public:
  DotVector(int size) {
    assertq(size >= 1, "There must be at least one element for DotVector");
    elements.resize(size);  
  }


  void load(Ptr<Float> input) {
    for (int i = 0; i < (int) elements.size(); ++i) {
      elements[i] = *input;  input += 16;
    }
  }


  void save(Ptr<Float> output) {
    for (int i = 0; i < (int) elements.size(); ++i) {
      *output = elements[i];  output += 16;
    }
  }


  /**
   * Calculate the dot product of current instance and `rhs`.
   *
   * All vector elements of the result will contain the same value.
   */
  void dot_product(Ptr<Float> rhs, Float &result) {
    Float tmp = 0;

    for (int i = 0; i < (int) elements.size(); ++i) {
      tmp = tmp + elements[i]*(*rhs);  rhs += 16;
    }

    rotate_sum(tmp, result);
  }

private:
  std::vector<Float> elements;
};


/**
 * Kernel for unit testing dot vectors of size 2
 */
template<int const N>
void check_dotvector(Ptr<Float> dst, Ptr<Float> a, Ptr<Float> result) {
  DotVector vec(N);
  vec.load(a);
  vec.save(dst);

  Float tmp = -2;  // Silly value for detection in unit test
  Float tmp2;

  vec.dot_product(a, tmp2);
  set_at(tmp, 0, tmp2);
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
  k.interpret();

  for (int i = 0; i < (int) a.size(); i++) {
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
  k.interpret();
  REQUIRE(result[0] == 16*N);
}


template<int const N>
void check_matrix_mult(Ptr<Float> dst, Ptr<Float> a, Ptr<Float> b) {
  int const DIM = 16*N;

  DotVector vec(N);
  Float result;

  For (Int y = 0, y < DIM, y++)
    vec.load(a + DIM*y);

    for (int x = 0; x < DIM; ++x) {
      Float tmp;
      vec.dot_product(b + DIM*x, tmp);

      set_at(result, x % 16, tmp);

      if (x % 16 == 15) {
        *dst = result;
        dst += 16;
      }
    }

  End
}



/**
 * Template parameters N is dimension of square matrix in block of 16 values
 */
template<int const N>
void test_matrix_multiplication() {
  int const SIZE = 16*N*16*N;

  SharedArray<float> a(SIZE);
  SharedArray<float> result(SIZE);

  float a_scalar[SIZE];
  for (int i = 0; i < SIZE; i++) {
    a_scalar[i] = 1;
  }

  float expected[SIZE];
  for (int i = 0; i < SIZE; i++) {
    expected[i] = -1;
  }
  matrix_mult_scalar(16*N, expected, a_scalar, a_scalar);

  for (int i = 0; i < SIZE; i++) {
    REQUIRE(expected[i] == 16*N);
  }

  auto k = compile(check_matrix_mult<N>);
  k.load(&result, &a, &a);

  //
  // Multiplication of empty input matrix
  //
  a.fill(0);
  result.fill(-1);
  k.interpret();

  for (int i = 0; i < SIZE; i++) {
    REQUIRE(result[i] == 0);
  }

  //
  // Square of input matrix containing all ones
  //
  a.fill(1);
  result.fill(-1);
  k.interpret();

  for (int i = 0; i < SIZE; i++) {
    REQUIRE(result[i] == 16*N);
  }

  //
  // Square of unit matrix
  //
  a.fill(0);
  for (int i = 0; i < 16*N; i++) {
    a[i + 16*N*i] = 1;
  }

  k.interpret();

  //dump(expected, SIZE, 16);
  //dump(result, 16*N);

  for (int x = 0; x < 16*N; x++) {
    for (int y = 0; y < 16*N; y++) {
      if (x == y) {
        REQUIRE(result[x + 16*N*y] == 1);
      } else {
        REQUIRE(result[x + 16*N*y] == 0);
      }
    }
  }


  //
  // Random values in array
  //
  for (int i = 0; i < SIZE; i++) {
    float val = (1.0f*(((float) (rand() % 200)) - 100.0f))/100.0f;  // Intention: values between -1 and 1
    a[i] = val;
    a_scalar[i] = val;
  }

  // b matrix needs to be transposed
  SharedArray<float> b(SIZE);

  for (int x = 0; x < 16*N; x++) {
    for (int y = 0; y < 16*N; y++) {
      b[y + 16*N*x] = a[x + 16*N*y];
    }
  }

  matrix_mult_scalar(16*N, expected, a_scalar, a_scalar);

  k.load(&result, &a, &b);
  k.interpret();

  float precision = 1e-5f;

  for (int i = 0; i < SIZE; i++) {
    REQUIRE(abs(result[i] - expected[i]) < precision);
  }
}

}  // anon namespace


TEST_CASE("Test Matrix algebra", "[matrix]") {
  using namespace V3DLib;
  Platform::use_main_memory(true);  // Run only interpreter and emulator for now


  SECTION("Check scalar matrix multiplication") {
    const int N = 16;  // Dimension of square matrix

    float a[N*N];
    float b[N*N];
    float c[N*N];

    // Init matrices
    for (int i = 0; i < N*N; i++) { a[i] =  1; }
    for (int i = 0; i < N*N; i++) { b[i] =  2; }
    for (int i = 0; i < N*N; i++) { c[i] = -1; }

    matrix_mult_scalar(N, c, a, b);
    
    for (int i = 0; i < N*N; i++) {
      REQUIRE(c[i] == 32);
    }
  }


  SECTION("Check rotate sum") {
    SharedArray<float> vec(16);
    vec.fill(0.3f);

    float expected = 0;
    for (int i = 0; i < (int) vec.size(); i++) {
      expected += vec[i];
    }

    REQUIRE(expected == 4.8f);

    SharedArray<float> result(16);
    result.fill(-1);

    auto k = compile(check_sum);

    k.load(&vec, &result);
    k.interpret();
    REQUIRE(result[0] == 4.8f);

    for (int i = 0; i < (int) vec.size(); i++) {
      vec[i] = 0.1f*((float ) (i + 1));
    }

    k.load(&vec, &result);
    k.interpret();
    REQUIRE(result[0] == 0.1f*(16*17/2));
    
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
    k.interpret();

    for (int i = 0; i < (int) result.size(); i++) {
      if (i == 0) {
        REQUIRE(result[i] == vec[i]);
      } else {
        REQUIRE(result[i] == -1);
      }
    }

    k.load(&vec, &result, 7);
    k.interpret();

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


  SECTION("Check matrix multiplication") {
    test_matrix_multiplication<1>();
    test_matrix_multiplication<2>();
  }


  Platform::use_main_memory(false);
}
