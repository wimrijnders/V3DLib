#include "support/support.h"
#include <iostream>
#include <complex>
#include <cmath>
#include <V3DLib.h>
#include "Support/Platform.h"

using namespace V3DLib;

namespace {

///////////////////////////////////////////////////////////////////////////////
// Support routines 
///////////////////////////////////////////////////////////////////////////////

/**
 * Load values into a 16-register
 */
void load_16vec(std::vector<int> &src, Int &dst, int unused_index = 15) {
  dst = unused_index;

  for (int n = 0; n < (int) src.size(); ++n) {
    Where (index() == n)
      dst = src[n];
    End
  }
}


///////////////////////////////////////////////////////////////////////////////
// FFT 
///////////////////////////////////////////////////////////////////////////////

const double PI = 3.1415926536;
typedef std::complex<double> cx;

unsigned int bitReverse(unsigned int x, int log2n) {
  int n = 0;

  for (int i=0; i < log2n; i++) {
    n <<= 1;
    n |= (x & 1);
    x >>= 1;
  }

  return n;
}


void fft(cx *a, cx *b, int log2n) {
  int n = 1 << log2n;
  for (int i=0; i < n; ++i) {
    b[bitReverse(i, log2n)] = a[i];
  }

  for (int s = 1; s <= log2n; ++s) {
    int m = 1 << s;
    int m2 = m >> 1;
    cx w(1, 0);
    cx wm(cos(-PI/m2), sin(-PI / m2));


    std::vector<int> k_index;
    std::vector<int> k_m2_index;

    for (int j = 0; j < m2; ++j) {
      for (int k = j; k < n; k += m) {
        k_index << k;
        k_m2_index << (k + m2);
      }
    }

    std::cout << "s " << s << ", k: ";
    for (int n = 0; n < (int) k_index.size(); ++n) {
      std::cout << k_index[n] << ", ";
    }
    std::cout << std::endl;

    std::cout << "s " << s << ", k + m2: ";
    for (int n = 0; n < (int) k_m2_index.size(); ++n) {
      std::cout << k_m2_index[n] << ", ";
    }
    std::cout << std::endl;


    for (int j = 0; j < m2; ++j) {
      for (int k = j; k < n; k += m) {
        cx t = w * b[k + m2];
        cx u = b[k];
        b[k] = u + t;
        b[k + m2] = u - t;
      }
      w *= wm;
    }
  }
}


//
// Kernel derived from scalar
//
// Result is returned in b
//
void fft_kernel(Complex::Ptr b, Int log2n) {
  b -= index();

  Int n = 1 << log2n;

  For (Int s = 1, s <= log2n, s++) 
    Int m = 1 << s;
    Int m2 = m >> 1;
    Complex w(1, 0);
    Complex wm(cos(Float(-1.0f)/(toFloat(m2)*2)), sin(Float(-1.0f)/(toFloat(m2)*2)));

    For (Int j = 0, j < m2, j++)
      For (Int k = j, k < n, k += m)
        Complex t = w * b[k + m2];
        Complex u = b[k];

        b[k]      = u + t;
        b[k + m2] = u - t;
      End 
      w *= wm;
    End
  End
}


}  // anon namespace


TEST_CASE("FFT [fft]") {
  cx a[] = { cx(0,0), cx(1,1), cx(3,3), cx(4,4), cx(4, 4), cx(3, 3), cx(1,1), cx(0,0) };

  //
  // Source: https://www.oreilly.com/library/view/c-cookbook/0596007612/ch11s18.html
  //
  SUBCASE("Scalar from example") {
    cx b[8];

    cx expected[] = {
      cx(16,16),
      cx(-4.82843,-11.6569),
      cx(0,0),
      cx(-0.343146,0.828427),
      cx(0,0),
      cx(0.828427,-0.343146),
      cx(0,0),
      cx(-11.6569,-4.82843)
    };

    fft(a, b, 3);

    float precision = 5.0e-5f;
    for (int i = 0; i < 8; ++i) {
      INFO("diff " << i << ": " << abs(b[i] - expected[i]));
      REQUIRE(abs(b[i] - expected[i]) < precision);
    }

    std::cout << "Scalar output: ";
    for (int i=0; i < 8; ++i) 
      std::cout << b[i] << ", ";
    std::cout << std::endl;
  }


  SUBCASE("Kernel derived from scalar") {
    if (Platform::has_vc4()) {
      warning("Test 'Kernel derived from scalar' works only on v3d, skipping");
      return;
    }

    int const NUM_POINTS = 8;
    int log2n = 3;  // Relates to NUM_POINTS

    Complex::Array aa(16);
    aa.fill(V3DLib::complex(0.0f, 0.0f));
    Complex::Array b(16);

    for (int i=0; i < NUM_POINTS; ++i) {
      aa.re()[i] = (float) a[i].real();
      aa.im()[i] = (float) a[i].imag();
    }

    // Perform bit reversal outside of kernel
    for (int i = 0; i < NUM_POINTS; ++i) {
      b[bitReverse(i, log2n)] = aa[i];
    }

    auto k = compile(fft_kernel);
    //k.pretty(true, nullptr, false);
    k.load(&b, log2n);
    k.call();
    std::cout << "Kernel output: " << b.dump() << std::endl;
  }
}


namespace {

std::vector<int> src_vec;

void vecload_kernel(Int::Ptr dst) {
  Int tmp = 0;
  load_16vec(src_vec, tmp);
  *dst = tmp;
}


void vecoffset_kernel(Int::Ptr dst, Int::Ptr src) {
  dst -= index();
  src -= index();

  Int s_index = 0;
  load_16vec(src_vec, s_index);

  Int tmp;
  tmp = *(src + s_index);
  tmp++;
  *(dst + s_index) = tmp;
}

}  // anon namespace


TEST_CASE("FFT Support [fft]") {
  SUBCASE("Test 16vec function") {
    std::vector<int> k_index = {0, 2, 4, 6};
    std::vector<int> k_m2_index = {1, 3, 5, 7};

    Int::Array result(16);

    {
      result.fill(-1);
      src_vec = k_index;
      auto k = compile(vecload_kernel);
      k.load(&result);
      k.interpret();
      std::cout << "16vec output: " << result.dump() << std::endl;

      for (int i = 0; i < (int) k_index.size(); ++i) {
        REQUIRE(k_index[i] == result[i]);
      }
    }

    {
      result.fill(-1);
      src_vec = k_m2_index;
      auto k = compile(vecload_kernel);
      k.load(&result);
      k.emu();
      std::cout << "16vec output: " << result.dump() << std::endl;

      for (int i = 0; i < (int) k_m2_index.size(); ++i) {
        REQUIRE(k_m2_index[i] == result[i]);
      }
    }
  }


  SUBCASE("Test 16vec as ptr offset") {
    std::vector<int> k_index = {0, 2, 4, 7};

    Int::Array a(16);
    for (int i = 0; i < (int) a.size(); ++i) {
      a[i] = i;
    }

    Int::Array result(16);
    result.fill(-1);

    src_vec = k_index;
    auto k = compile(vecoffset_kernel);
    k.load(&result, &a);
    k.call();

    std::cout << "16vec output: " << result.dump() << std::endl;
  }
}
