#include "support/support.h"
#include <iostream>
#include <complex>
#include <cmath>
#include <V3DLib.h>

using namespace V3DLib;

namespace {

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
  const cx J(0, 1);

  int n = 1 << log2n;
  for (int i=0; i < n; ++i) {
    b[bitReverse(i, log2n)] = a[i];
  }

  for (int s = 1; s <= log2n; ++s) {
    int m = 1 << s;
    int m2 = m >> 1;
    cx w(1, 0);
    cx wm = exp(-J * (PI / m2));
    for (int j=0; j < m2; ++j) {
      for (int k=j; k < n; k += m) {
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
  Int n = 1 << log2n;
  Complex /*const*/ J(0, 1);

  For (Int s = 1, s <= log2n, s++) 
    Int m = 1 << s;
    Int m2 = m >> 1;
    Complex w(1, 0);
/*
    cx wm = exp(-J * (PI / m2));
*/
    For (Int j = 0, j < m2, j++)
      For (Int k = j, k < n, k += m)
        cx t = w * b[k + m2];
        cx u = b[k];
        b[k] = u + t;
        b[k + m2] = u - t;
      End 
/*
      w *= wm;
*/
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
    for (int i=0; i < 8; ++i) {
      INFO("diff " << i << ": " << abs(b[i] - expected[i]));
      REQUIRE(abs(b[i] - expected[i]) < precision);
    }

    for (int i=0; i < 8; ++i) 
      std::cout << b[i] << "\n";
  }


  SUBCASE("Kernel derived from scalar") {
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
    k.load(&b, log2n);
    std::cout << b.dump() << std::endl;
  }
}


