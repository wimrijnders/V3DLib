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
//#include "Source/gather.h"

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
 * Determine and check the incremental step for the items in the vector
 *
 * at most 16 consecutive values are handled, starting from offset
 *
 * @return step  incremental step value if found, -1 otherwise
 */
int determine_step(std::vector<int> const &src, int offset = 0) {
  assert(src.size() >= 2);  // Handle other cases when we get to them

  int size = (int) src.size() - offset;
  if (size > 16) {
    size = 16;
  }
  
  int step = src[offset + 1] - src[offset];

  bool verified = true;
  for (int n = 1; n < size; ++n) {
    if (src[offset] + n*step != src[offset + n]) {
      verified = false;
      break;
    }
  }

  if (verified) {
    return step;
  } else {
    debug("determine_step() fail");
    return -1;
  }
}



/**
 * Load values into a 16-register
 */
void load_16vec(std::vector<int> const &src, Int &dst, int offset = 0, int unused_index = 16) {
  assert(offset < (int) src.size());

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
  int j, int n, int m, int m2) {
  k_index.clear();
  k_m2_index.clear();

  for (int k = j; k < n; k += m) {
    k_index << k;
    k_m2_index << (k + m2);
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


struct OffsetItem {
  int s;
  std::vector<int> k_index;
  std::vector<int> k_m2_index;

  std::string dump() {
    std::string ret;

    ret << "s: " << s << "\n     k: ";
    for (int n = 0; n < (int) k_index.size(); ++n) {
      ret << k_index[n] << ", ";
    }

    ret << "\n  k+m2: ";

    for (int n = 0; n < (int) k_m2_index.size(); ++n) {
      ret << k_m2_index[n] << ", ";
    }

    return ret;
  }
};

using Offsets = std::vector<OffsetItem>;


/**
 * Precalculate the offsets to use for FFT
 */
Offsets prepare_offsets(int s, int m) {
  Offsets ret;

  int m2 = m >> 1;
  int n = 1 << fft_context.log2n;

  for (int j = 0; j < m2; j++) {
    OffsetItem item;
    item.s = s;

    create_dft_offsets(item.k_index, item.k_m2_index, j, n, m, m2);
    assert(item.k_index.size() == item.k_m2_index.size());

    ret << item;
  }

  return ret;
}


/**
 * Interim construct to combine consecutive offset rows
 */
struct CombinedOffsets {
  int step    = -1;
  std::vector<std::vector<int> *> k_vec;
  std::vector<std::vector<int> *> k_m2_vec;

  int k_count() const { return (m_k_count < 1)?1:m_k_count; }

private:
  int m_k_count = -1;

public:

  CombinedOffsets(Offsets &offsets, int j) {
    assert(!offsets.empty());
    assert(0 <= j && j < (int) offsets.size());
    int cur_size = (int) offsets[j].k_index.size();
    m_k_count = 16/cur_size;

    if (j + m_k_count > (int) offsets.size()) {
      std::string msg = "CombinedOffsets() size check failed: ";
      msg << "j = " << j << ", k_count: " << m_k_count << ", size: " << offsets.size();
      assertq(false, msg);
    }


    // All handled rows should have same s-index and size
    for (int k = 1; k < m_k_count; k++) {
//      int cur_size = 16/m_k_count;
      assert(offsets[j].s == offsets[j + k].s);
      assert((int) offsets[j + k].k_index.size() == cur_size);
    }

    // Load in vectors to handle
    if (m_k_count <= 1) {
      k_vec    << &offsets[j].k_index;
      k_m2_vec << &offsets[j].k_m2_index;
    } else {
      for (int k = 0; k < m_k_count; k++) {
        k_vec << &offsets[j + k].k_index;
      }

      for (int k = 0; k < m_k_count; k++) {
        k_m2_vec << &offsets[j+k].k_m2_index;
      }
    }

    if (m_k_count == 16) {
      step = 0;
    } else {
      step = determine_step(*k_vec[0]);
      assert(step != -1);

      for (int k = 1; k < m_k_count; k++) {
        assert(step == determine_step(*k_vec[k]));
        assert(step == determine_step(*k_m2_vec[k]));
      }
    }
/*
    {
      std::string msg;
      msg << "step for j = " << j << ": " << step;
      debug(msg);
    }
*/

  }


  int size() {
    assert(!k_vec.empty());
    return (int) k_vec[0]->size();
  }


  /**
   * Construct mult factor to use
   */
  void init_mult_factor(Complex &w_off, Complex &wm) {
    if (k_count() == 1) return;

    int width = 16/k_count();
    for (int i = 1; i < k_count(); i++) {
      Int offset = index() - width*i;
      Where (offset >= 0)
        w_off *= wm;
      End
    }
  }


  /**
   * Construct reg offsets to use
   */
  void init_k(Int &k_off, Int &k_m2_off, int offset = 0) {
    assert(step != -1);
    k_off     = (*k_vec[0])[offset];
    k_m2_off  = (*k_m2_vec[0])[offset];
    k_off    += index()*step;
    k_m2_off += index()*step;

    if (k_count() == 1) return;

    if (k_count() == 16) {
      // There is no step; load values in directly
      std::vector<int> tmp;
      for (int i = 0; i < m_k_count; i++) {
        tmp << *k_vec[i];
      }
      assert(tmp.size() == 16);
      load_16vec(tmp, k_off);

      tmp.clear();
      for (int i = 0; i < m_k_count; i++) {
        tmp << *k_m2_vec[i];
      }
      assert(tmp.size() == 16);
      load_16vec(tmp, k_m2_off);
      return;
    }

    int width = 16/k_count();
    for (int i = 1; i < k_count(); i++) {
      Int offset = index() - width*i;
      Where (offset >= 0)
        k_off     = (*k_vec[i])[0];
        k_off    += offset*step;
        k_m2_off  = (*k_m2_vec[i])[0];
        k_m2_off += offset*step;
      End
    }
  }
};


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

  auto calc_step = [] (Complex::Ptr &b_k, Complex::Ptr &b_k_m2, Complex &w) {
    Complex t = w * (*b_k_m2);
    Complex u = *b_k;

    *b_k    = u + t;
    *b_k_m2 = u - t;
  };


  for (int s = 1; s <= fft_context.log2n; s++) {
    int m = 1 << s;
    Complex w(1, 0);

    float phase = -1.0f/((float) m);
    Complex wm(V3DLib::cos(phase), V3DLib::sin(phase));  // library sin/cos may be more efficient here

    auto offsets = prepare_offsets(s, m);

    for (int j = 0; j < (int) offsets.size(); j++) {
      auto &item = offsets[j];
/*
      if (item.k_index.size() < 4) {
        std::cout << "j: " << j << ", item:" << item.dump();
        std::cout << std::endl;
      }
*/
      int cur_size = (int) item.k_index.size();
      int k_count = 16/cur_size;

      int last_k_count  = -1;
      int k_count_hits  = 0;
      int k_count_total = 0;
      Complex w_off;

      if ((j + k_count <= (int) offsets.size())) {
        CombinedOffsets co(offsets, j);

        if (last_k_count != co.k_count()) {
          w_off = w;
          co.init_mult_factor(w_off, wm);
          last_k_count = co.k_count();
          k_count_hits++;
        }
        k_count_total++;

        for (int offset = 0; offset < (int) co.size(); offset += 16) {
          Int k_off     = 0;
          Int k_m2_off  = 0;
          co.init_k(k_off, k_m2_off, offset);

          Complex::Ptr b_k    = b + k_off;
          Complex::Ptr b_k_m2 = b + k_m2_off;

          calc_step(b_k, b_k_m2, w_off);
        }
       
        // Extra loop offset for combined rows
        for (int k = 1; k < co.k_count(); k++) {
          w *= wm; 
          j++;
        }
      } else {
        // Fallback option for tiny FFTs, which are not big enough to combine values into 16-vecs
        //debug("Fallback!");
        auto &k_index    = offsets[j].k_index;
        auto &k_m2_index = offsets[j].k_m2_index;

        Complex::Ptr b_k    = b;
        Complex::Ptr b_k_m2 = b;
        load_16ptr(k_index, b_k, devnull);
        load_16ptr(k_m2_index, b_k_m2, devnull);

        calc_step(b_k, b_k_m2, w);
      }

      // Following always 'k_count hits: 1, total: 1'
/*
      {
        std::string msg;
        msg << "k_count hits: " << k_count_hits << ", total: " << k_count_total;
        debug(msg);
      }
*/

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
    int log2n = 9;
    int Dim = 1 << log2n;

    int size = Dim;
    if (size < 16) size = 16;

    Float::Array a(size);
    for (int c = 0; c < Dim; ++c) {
      a[c] = wavelet_function(c, Dim);
    }

    // Run scalar FFT for comparison
    {
      cx a[Dim];
      for (int c = 0; c < Dim; ++c) {
        a[c] = cx(wavelet_function(c, Dim), 0.0f);
      }
      cx scalar_result[Dim];

      Timer timer1("scalar FFT run time");
      fft(a, scalar_result, log2n);
      timer1.end();

      // Perhaps TODO: compare scalar result with kernel output
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

    float precision = 5.0e-4f;  // adequate for log2n <= 8:  5.0e-5f;
    for (int i = 0; i < Dim; ++i) {
      float diff = (result_float[0][i] - result[i].to_complex()).magnitude();
      INFO("diff " << i << ": " << diff);
      REQUIRE(diff < precision);
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
