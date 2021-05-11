#include "support/support.h"
#include <iostream>
#include <complex>
#include <cmath>
#include <V3DLib.h>
#include "Support/Platform.h"
#include "Support/Timer.h"
#include "Support/pgm.h"
#include "support/dft_support.h"
#include "Kernels/Matrix.h"

using namespace V3DLib;

namespace {

///////////////////////////////////////////////////////////////////////////////
// Support routines 
///////////////////////////////////////////////////////////////////////////////

typedef std::complex<double> cx;

cx operator-(cx const &a, complex const &b) {
  return cx(a.real() - b.re(), a.imag() - b.im());
}

/**
 * Load values into a 16-register
 */
void load_16vec(std::vector<int> const &src, Int &dst, int offset = 0, int unused_index = 16) {
  assert(offset < (int) src.size());

  if (src.size() >= 16) {
    assert(offset + 16 <= (int) src.size());
    // Find linear sequence, if any

    int step = src[offset + 1] - src[offset];
    bool verified = true;
    for (int n = 1; n < 16; ++n) {
      if (src[offset] + n*step != src[offset + n]) {
        verified = false;
        break;
      }
    }

    if (verified) {
      //debug("Verified success");
      dst = src[offset];
      dst += index()*step;
      return;
    }

    debug("Verified fail");
  }
 
  dst = unused_index;

  int size = (int) src.size() - offset;
  if (size > 16) size = 16;

  for (int n = 0; n < size; ++n) {
    Where (index() == n)
      dst = src[n + offset];
    End
  }
}


template<typename t, typename Ptr>
void load_16ptr(std::vector<t> const &src_vec, Ptr &ptr, Ptr &devnull, int offset = 0) {
  int unused = -1;
  Int s_index = 0;
  load_16vec(src_vec, s_index, offset, unused);

  ptr += s_index;

  if (src_vec.size() < 16) {
    Where (s_index == unused)
      ptr = devnull;
    End
  }
}


void create_dft_offsets(
  std::vector<int> &k_index,
  std::vector<int> &k_m2_index,
  int j, int n, int m, int m2,
  bool debug_output = false) {
  k_index.clear();
  k_m2_index.clear();

  for (int k = j; k < n; k += m) {
    k_index << k;
    k_m2_index << (k + m2);
  }

  // Debug output
  if (debug_output && k_index.size() < 16) {
    std::cout << "j: " << j << ", k: ";
    for (int n = 0; n < (int) k_index.size(); ++n) {
      std::cout << k_index[n] << ", ";
    }
    std::cout << std::endl;

    std::cout << "j: " << j << ", k + m2: ";
    for (int n = 0; n < (int) k_m2_index.size(); ++n) {
      std::cout << k_m2_index[n] << ", ";
    }
    std::cout << std::endl;
  }
}

///////////////////////////////////////////////////////////////////////////////
// FFT 
///////////////////////////////////////////////////////////////////////////////

const double PI = 3.1415926536;

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


struct {
  int log2n;
} fft_context;


/**
 * Kernel derived from scalar
 *
 * The result is returned in b.
 *
 * This kernel works with `emu()` and on `v3d` with `call()`, *not* with `interpret()` or on `vc4`.
 * So be it. Life sucks sometimes.
 */
void fft_kernel(Complex::Ptr b, Complex::Ptr devnull) {
  b -= index();

  int n = 1 << fft_context.log2n;

  for (int s = 1; s <= fft_context.log2n; s++) {
    int m = 1 << s;
    int m2 = m >> 1;
    Complex w(1, 0);

    //Complex wm(cos(Float(-1.0f)/(toFloat(m2)*2)), sin(Float(-1.0f)/(toFloat(m2)*2)));
    float phase = -1.0f/((float) (m2*2));
    Complex wm(V3DLib::cos(phase), V3DLib::sin(phase));  // library sin/cos may be more efficient here

    std::vector<int> k_index;
    std::vector<int> k_m2_index;

    for (int j = 0; j < m2; j++) {
      create_dft_offsets(k_index, k_m2_index, j, n, m, m2, true);
      assert(k_index.size() == k_m2_index.size());

      for (int offset = 0; offset < (int) k_index.size(); offset += 16) {
        Complex::Ptr b_k    = b;
        Complex::Ptr b_k_m2 = b;
        load_16ptr(k_index, b_k, devnull, offset);
        load_16ptr(k_m2_index, b_k_m2, devnull, offset);

        Complex t = w * (*b_k_m2);
        Complex u = *b_k;

        *b_k    = u + t;
        *b_k_m2 = u - t;
      }

      w *= wm;
    }
  }
}


}  // anon namespace


TEST_CASE("FFT test with scalar [fft]") {
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

/*
    std::cout << "Scalar output: ";
    for (int i=0; i < 8; ++i) 
      std::cout << b[i] << ", ";
    std::cout << std::endl;
*/
  }


  SUBCASE("Kernel derived from scalar") {
    if (Platform::has_vc4()) {
      warning("Test 'Kernel derived from scalar' works only on v3d, skipping");
      return;
    }

    int const NUM_POINTS = 8;
    int log2n = 3;  // Relates to NUM_POINTS

    cx scalar_result[8];
    fft(a, scalar_result, log2n);

    Complex::Array aa(16);
    aa.fill(V3DLib::complex(0.0f, 0.0f));
    Complex::Array result(16);
    Complex::Array devnull(16);

    for (int i=0; i < NUM_POINTS; ++i) {
      aa.re()[i] = (float) a[i].real();
      aa.im()[i] = (float) a[i].imag();
    }

    // Perform bit reversal outside of kernel
    for (int i = 0; i < NUM_POINTS; ++i) {
      result[bitReverse(i, log2n)] = aa[i];
    }

    fft_context.log2n = log2n;
    auto k = compile(fft_kernel, V3D);
    //k.pretty(true, nullptr, false);
    k.load(&result, &devnull);
    k.call();
    //std::cout << "Kernel output: " << result.dump() << std::endl;

    float precision = 5.0e-5f;
    for (int i = 0; i < NUM_POINTS; ++i) {
      INFO("diff " << i << ": " << abs(scalar_result[i] - result[i].to_complex()));
      REQUIRE(abs(scalar_result[i] - result[i].to_complex()) < precision);
    }
  }
}


TEST_CASE("FFT test with DFT [fft]") {
  SUBCASE("Compare FFT and DFT output") {
    int log2n = 6;
    int Dim = 1 << log2n;

    int size = Dim;
    if (size < 16) size = 16;

    Float::Array a(size);
    for (int c = 0; c < Dim; ++c) {
      a[c] = wavelet_function(c, Dim);
    }

    // Run DFT for comparison
    Complex::Array2D result_float;
    {
      Timer timer1("DFT compile time");
      auto k = compile(kernels::dft_inline_decorator(a, result_float), V3D);
      timer1.end();
      std::cout << "DFT kernel size: " << k.v3d_kernel_size() << std::endl;

      Timer timer2("DFT run time");
      k.load(&result_float, &a);
      //k.setNumQPUs(1);
      k.call();
      timer2.end();

      //std::cout << "DFT result: " << result_float.dump() << std::endl;
    }


    Complex::Array result(size);
    result.fill(V3DLib::complex(0.0f, 0.0f));

    Complex::Array devnull(16);

    // Perform bit reversal outside of kernel
    for (int i = 0; i < Dim; ++i) {
      result[bitReverse(i, log2n)] = complex(a[i], 0.0f);
    }

    fft_context.log2n = log2n;

    Timer timer1("FFT compile time");
    auto k = compile(fft_kernel, V3D);
    timer1.end();
    std::cout << "FFT kernel size: " << k.v3d_kernel_size() << std::endl;

    Timer timer2("FFT run time");
    k.load(&result, &devnull);
    k.call();
    timer2.end();

    //std::cout << "FFT result: " << result.dump() << std::endl;

    {
      float real_result[Dim];
      for (int c = 0; c < Dim; ++c) {
        real_result[c] = result[c].to_complex().magnitude();
      }


      char const *file_prefix = "fft";
      std::string filename = "obj/test/";
      filename << file_prefix << "_result.pgm";

      PGM pgm(Dim, 100);
      pgm.plot(real_result, Dim)
         .save(filename.c_str());
    }

    float precision = 5.0e-5f;
    for (int i = 0; i < Dim; ++i) {
      INFO("diff " << i << ": " << (result_float[0][i] - result[i].to_complex()).magnitude());
      REQUIRE((result_float[0][i] - result[i].to_complex()).magnitude() < precision);
    }
  }
}


namespace {

std::vector<int> src_vec;

void vecload_kernel(Int::Ptr dst) {
  Int tmp = 0;
  load_16vec(src_vec, tmp);
  *dst = tmp;
}


void vecoffset_kernel(Int::Ptr dst, Int::Ptr src, Int::Ptr devnull) {
  dst -= index();
  src -= index();

  load_16ptr(src_vec, src, devnull);
  load_16ptr(src_vec, dst, devnull);

  Int tmp;
  tmp = *src;
  tmp++;
  *dst = tmp;
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
      k.call();

      //std::cout << "16vec output: " << result.dump() << std::endl;
      for (int i = 0; i < (int) k_index.size(); ++i) {
        REQUIRE(k_index[i] == result[i]);
      }
    }

    {
      result.fill(-1);
      src_vec = k_m2_index;
      auto k = compile(vecload_kernel);
      k.load(&result);
      k.call();

      //std::cout << "16vec output: " << result.dump() << std::endl;
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

    Int::Array devnull(16);
    Int::Array result(16);
    result.fill(0);

    std::vector<int> expected = { 1, 0, 3, 0, 5, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0};

    src_vec = k_index;
    auto k = compile(vecoffset_kernel);
    k.load(&result, &a, &devnull);
    k.call();

    //std::cout << "16vec output: " << result.dump() << std::endl;

    REQUIRE(expected.size() == result.size());
    for (int i = 0; i < (int) expected.size(); ++i) {
      REQUIRE(expected[i] == result[i]);
    }
  }
}
