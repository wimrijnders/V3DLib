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
 * Paranoia: check that steps are valid for the entire vector
 *
 * e.g. for width == 8, the following is valid (fictional example):
 *
 *  { 2, 4, 6, 8, 10, 12, 14, 16, 32, 34, 36, 38, 40, 42, 44, 46}
 *
 *  - i.e. 2 subranges of size 8 in which the step is valid
 *
 */
bool verify_step(std::vector<int> const &src, int step, int width) {
  assert(src.size() >= 2);  // No point in determining step for size 1
  assert(width <= 16);
  assert( (int) src.size() % width == 0);

  bool verified = true;
  
  for (int i = 0; i < (int) src.size(); i += width) {
    for (int n = 1; n < width; ++n) {
      if (src[i] + n*step != src[i+ n]) {
        verified = false;
        break;
      }
    }
  }

  if (!verified) {
    debug("verify_step() fail");
  }

  return verified;
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
  int j;
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
    item.j = j;

    create_dft_offsets(item.k_index, item.k_m2_index, j, n, m, m2);
    assert(item.k_index.size() == item.k_m2_index.size());

    ret << item;
  }

  return ret;
}


/**
 * Generate all indexes for the FFT
 */
std::vector<Offsets> prepare_fft_indexes(int log2n) {
  std::vector<Offsets> ret;

  for (int s = 1; s <= log2n; s++) {
    int m = 1 << s;
    ret << prepare_offsets(s, m);
  }

  return ret;
}



/**
 * Construct mult factor to use
 */
void init_mult_factor(Complex &w_off, Complex &wm, int k_count) {
  if (k_count == 1) return;

  int width = 16/k_count;
  for (int i = 1; i < k_count; i++) {
    Int offset = index() - width*i;
    Where (offset >= 0)
      w_off *= wm;
    End
  }
}


void init_k(
  Int &k_off, Int &k_m2_off,
  std::vector<int> const &k_tmp,
  std::vector<int> const &k_m2_tmp,
  int step, int k_count) {

  if (k_count == 16) {
    // There is no step; load values in directly
    load_16vec(k_tmp, k_off);
    load_16vec(k_m2_tmp, k_m2_off);
    return;
  }


  assert(step != -1);
  k_off     = k_tmp[0];
  k_m2_off  = k_m2_tmp[0];
  k_off    += index()*step;
  k_m2_off += index()*step;

  if (k_count == 1) return;

  int width = 16/k_count;
  for (int i = 1; i < k_count; i++) {
    Int offset2 = index() - width*i;

    Where (offset2 >= 0)
      k_off     = k_tmp[i*width];
      k_off    += offset2*step;
      k_m2_off  = k_m2_tmp[i*width];
      k_m2_off += offset2*step;
    End
  }
}


/**
 * Interim construct to combine consecutive offset rows
 */
struct CombinedOffsets {
  std::vector<int> k_vec;
  std::vector<int> k_m2_vec;


  CombinedOffsets(Offsets &offsets, int j) {
    assert(!offsets.empty());
    assert(0 <= j && j < (int) offsets.size());
    m_lines   = (int) offsets.size();
    m_size    = (int) offsets[j].k_index.size();
    m_k_count = 16/m_size;

    if (!valid_index(j)) {
/*
      std::string msg = "CombinedOffsets() size check failed: ";
      msg << "j = " << j << ", k_count: " << m_k_count << ", size: " << offsets.size();
      assertq(false, msg);
*/
      return;
    }


    // All handled rows should have same s-index and size
    for (int k = 1; k < k_count(); k++) {
      assert(offsets[j].s == offsets[j + k].s);
      assert((int) offsets[j + k].k_index.size() == m_size);
    }

    int width = 16/k_count();

    // Load in vectors to handle
    for (int k = 0; k < k_count(); k++) {
      k_vec    << offsets[j + k].k_index;
      k_m2_vec << offsets[j + k].k_m2_index;
    }

    if (k_count() == 16) {
      m_step = 0;
    } else {
      m_step = k_vec[1] - k_vec[0];
      assert(m_step > 0);
      assert(verify_step(k_vec,    m_step, width));
      assert(verify_step(k_m2_vec, m_step, width));
    }
/*
    {
      std::string msg;
      msg << "step for j = " << j << ": " << m_step;
      debug(msg);
    }
*/

  }


  int k_count() const {
    assert(m_k_count != -1);
    return (m_k_count < 1)?1:m_k_count;
  }

  int step() const { return m_step; }


  bool valid_index(int j) const { return (j + k_count() <= m_lines); }

  int size() const {
    assert(m_size != -1);
    return m_size;
  }


  /**
   * Construct reg offsets to use
   */
  void init_k(Int &k_off, Int &k_m2_off, int offset = 0) {
    std::vector<int> k_tmp;
    std::vector<int> k_m2_tmp;

    if (k_count() == 16) {
      assert(k_vec.size() == 16);
      assert(k_m2_vec.size() == 16);
      k_tmp    = k_vec;
      k_m2_tmp = k_m2_vec;
    } else {
      assert(offset + 16 <= (int) k_vec.size());
      assert(offset + 16 <= (int) k_m2_vec.size());

      for (int k = 0; k < 16; k++) {
        k_tmp    << k_vec[offset + k];
        k_m2_tmp << k_m2_vec[offset + k];
      }
    }

    ::init_k(k_off, k_m2_off, k_tmp, k_m2_tmp, m_step, k_count());
  }


private:
  int m_k_count  = -1;
  int m_step     = -1;
  int m_lines    = -1;
  int m_size     = -1;
};


struct Indexes16 {
  int s            = -1;
  int j            = -1;
  int step         = -1;
  int k_count      = -1;
  bool valid_index = false;
  std::vector<int> k_16;
  std::vector<int> k_m2_16;

  Indexes16(CombinedOffsets const &co, int in_s, int in_j) {
    s = in_s;
    j = in_j;
    step        = co.step();
    k_count     = co.k_count();
    valid_index = co.valid_index(j);
  }

  Indexes16(CombinedOffsets const &co, int in_s, bool not_valid) {
    assert(not_valid == false);
    s = in_s;
    k_count     = co.k_count();
    valid_index = false;
  }
};


/**
 * Rearrange the fft index into 16-vectors
 */
std::vector<Indexes16> indexes_to_16vectors(std::vector<Offsets> const &fft_indexes) {
  std::vector<Indexes16> ret;

  for (int i = 0; i < (int) fft_indexes.size(); i++) {
    int s = i + 1;
    auto offsets = fft_indexes[i];

    for (int j = 0; j < (int) offsets.size(); j++) {
      CombinedOffsets co(offsets, j);

      if (co.valid_index(j)) {
        if (co.k_count() == 1) {
          assert((int) co.k_vec.size() % 16 == 0);
          assert((int) co.k_m2_vec.size() % 16 == 0);

          for (int offset = 0; offset < (int) co.size(); offset += 16) {
            Indexes16 item(co, s, j);

            for (int k = 0; k < 16; k++) {
              item.k_16    << co.k_vec[offset + k];
              item.k_m2_16 << co.k_m2_vec[offset + k];
            }

            ret << item;
          }
        } else {
          assert(co.k_vec.size() == 16);
          assert(co.k_m2_vec.size() == 16);

          Indexes16 item(co, s, j);
          item.k_16    = co.k_vec;
          item.k_m2_16 = co.k_m2_vec;

          ret << item;
        }
      } else {
        ret << Indexes16(co, s, false);
      }

      j += (co.k_count() - 1);
    }
  }

  //
  // Check assumption:
  //   - if one item does not have a valid index, then all items are not valid
  //   - conversely, if one item is valid, then all items are valid
  //
  int valid_count = 0;
  for (int i = 0; i < (int) ret.size(); i++) {
    if (ret[i].valid_index) valid_count++;
  }

  if (!(valid_count == 0 || valid_count == (int) ret.size())) {
    std::string msg = "Valid count failed: ";
    msg << valid_count << " out of " << ret.size() << " valid";
    assertq(false, msg);
  }
  

  return ret;
}


void fft_step(Complex::Ptr &b_k, Complex::Ptr &b_k_m2, Complex &w) {
    Complex t = w * (*b_k_m2);
    Complex u = *b_k;

    *b_k    = u + t;
    *b_k_m2 = u - t;
}


/**
 * Fallback option for tiny FFTs with dimension <= 8
 *
 * These are not big enough to combine values into 16-vecs
 *
 * This is a direct translation of the scalar FFT.
 *
 * It actually works for larger dimensions, but the performance will be horrid;
 * only one value is used in the 16-vec registers.
 */
void tiny_fft(Complex::Ptr &b, Complex::Ptr &devnull, std::vector<Offsets> const &fft_indexes) {
  debug("Fallback!");

  for (int i = 0; i < (int) fft_indexes.size(); i++) {
    int s = i + 1;

    Complex w(1, 0);

    float phase = -1.0f/((float) (1 << s));
    Complex wm(V3DLib::cos(phase), V3DLib::sin(phase));  // library sin/cos may be more efficient here

    auto offsets = fft_indexes[i];

    for (int j = 0; j < (int) offsets.size(); j++) {
      Complex w_off;

      CombinedOffsets co(offsets, j);
      assert(!co.valid_index(j));  // Check if this is really for tiny fft

      auto &k_index    = offsets[j].k_index;
      auto &k_m2_index = offsets[j].k_m2_index;

      Complex::Ptr b_k    = b;
      Complex::Ptr b_k_m2 = b;
      load_16ptr(k_index, b_k, devnull);
      load_16ptr(k_m2_index, b_k_m2, devnull);

      fft_step(b_k, b_k_m2, w);

      w *= wm;
    }
  }
}


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

  auto fft_indexes = prepare_fft_indexes(fft_context.log2n);
  std::vector<Indexes16> vectors16 = indexes_to_16vectors(fft_indexes);
  assert(!vectors16.empty());

  if (!vectors16[0].valid_index) {
    tiny_fft(b, devnull, fft_indexes);
    return;
  }

  int last_s = -1;
  Complex w(0, 0);
  Complex wm(0, 0);

  int last_k_count  = -1;
  int last_j        = -1;
  Complex w_off;

  for (int i = 0; i < (int) vectors16.size(); i++) {
    auto &item = vectors16[i];

    if (last_s != item.s) {
      w = Complex(1, 0);

      float phase = -1.0f/((float) (1 << item.s));
      wm = Complex(V3DLib::cos(phase), V3DLib::sin(phase));  // library sin/cos may be more efficient here
   
      last_s = item.s; 
    }

    assert(item.valid_index);

    if (last_j != item.j) {
      last_k_count = -1;
      last_j = item.j;
    }

    if (last_k_count != item.k_count) {
      w_off = w;
      init_mult_factor(w_off, wm, item.k_count);
      last_k_count = item.k_count;
    }

    Int k_off     = 0;
    Int k_m2_off  = 0;
    init_k(k_off, k_m2_off, item.k_16, item.k_m2_16, item.step, item.k_count);

    Complex::Ptr b_k    = b + k_off;
    Complex::Ptr b_k_m2 = b + k_m2_off;

    fft_step(b_k, b_k_m2, w_off);

    if (i + 1 < (int) vectors16.size()) {
      auto &next_item = vectors16[i + 1];
      if (item.j != next_item.j) {
        // Extra loop offset for combined rows
        for (int k = 1; k < item.k_count; k++) {
          w *= wm; 
        }

        w *= wm;
      }
    }
  }

#if 0
  for (int i = 0; i < (int) fft_indexes.size(); i++) {
    int s = i + 1;

    Complex w(1, 0);

    float phase = -1.0f/((float) (1 << s));
    Complex wm(V3DLib::cos(phase), V3DLib::sin(phase));  // library sin/cos may be more efficient here

    auto offsets = fft_indexes[i];

    for (int j = 0; j < (int) offsets.size(); j++) {
/*
      auto &item = offsets[j];
      if (item.k_index.size() < 4) {
        std::cout << "j: " << j << ", item:" << item.dump();
        std::cout << std::endl;
      }
*/
      int last_k_count  = -1;
      Complex w_off;

      CombinedOffsets co(offsets, j);
      assert(co.valid_index(j));

      if (last_k_count != co.k_count()) {
        w_off = w;
        init_mult_factor(w_off, wm, co.k_count());
        last_k_count = co.k_count();
      }

      for (int offset = 0; offset < (int) co.size(); offset += 16) {
        Int k_off     = 0;
        Int k_m2_off  = 0;
        co.init_k(k_off, k_m2_off, offset);

        Complex::Ptr b_k    = b + k_off;
        Complex::Ptr b_k_m2 = b + k_m2_off;

        fft_step(b_k, b_k_m2, w_off);
      }
       
      // Extra loop offset for combined rows
      for (int k = 1; k < co.k_count(); k++) {
        w *= wm; 
        j++;
      }

      w *= wm;
    }
  }
#endif
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
