#include "support/support.h"
#include <iostream>
#include <cmath>
#include <V3DLib.h>
#include "Support/Platform.h"
#include "Support/Timer.h"
#include "Support/pgm.h"
#include "support/dft_support.h"
#include "Kernels/Matrix.h"
#include "Source/gather.h"

using namespace V3DLib;

namespace {

///////////////////////////////////////////////////////////////////////////////
// Support routines 
///////////////////////////////////////////////////////////////////////////////

float max_mag_diff(cx const *scalar_result, Complex::Array const &result) {
  float ret = 0;

  for (int i = 0; i < (int) result.size(); i++) {
    float diff = (float) abs(scalar_result[i] - result[i].to_complex());
    if (ret < diff)
      ret = diff;
  }

  return ret;
}


void check_result1(cx const *expected, Complex::Array2D const &result, int Dim, float precision) {
  REQUIRE(precision > 0.0f);
  for (int i = 0; i < Dim; ++i) {
    float diff = (float) abs(expected[i] - result[0][i]);
    INFO("diff " << i << ": " << diff);
    REQUIRE(diff < precision);
  }
}


void check_result2(cx const *expected, Complex::Array const &result, int Dim, float precision) {
  REQUIRE(precision > 0.0f);
  for (int i = 0; i < Dim; ++i) {
    float diff = (float) abs(expected[i] - result[i].to_complex());
    INFO("diff " << i << ": " << diff);
    REQUIRE(diff < precision);
  }
}

/*
void check_result(Complex::Array2D const &expected, Complex::Array const &result, int Dim, float precision) {
  REQUIRE(precision > 0.0f);
  for (int i = 0; i < Dim; ++i) {
    float diff = (expected[0][i] - result[i].to_complex()).magnitude();
    INFO("diff " << i << ": " << diff);
    REQUIRE(diff < precision);
  }
}
*/


#ifdef DEBUG

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

#endif  // DEBUG



/**
 * Load values into a 16-register
 */
void load_16vec(Int &dst, std::vector<int> const &src, int offset = 0, int unused_index = 16) {
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
  load_16vec(s_index, src_vec, offset, unused);

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
  int Dim = 1 << log2n;
  for (int i=0; i < Dim; ++i) {
    b[bitReverse(i, log2n)] = a[i];
  }

  for (int s = 1; s <= log2n; ++s) {
    int m = 1 << s;
    int m2 = m >> 1;
    cx w(1, 0);
    cx wm(cos(-M_PI/m2), sin(-M_PI/m2));

    for (int j = 0; j < m2; ++j) {
      for (int k = j; k < Dim; k += m) {
        cx t = w * b[k + m2];
        cx u = b[k];
        b[k] = u + t;
        b[k + m2] = u - t;
      }

      w *= wm;
    }
  }
}


class Vec16 : private std::vector<int> {
  using Parent = std::vector<int>;

public:
  using Parent::operator[];
  using Parent::size;

  Parent const &parent() const { return *this; }

  Vec16() {
    resize(16);
    for (int i = 0; i < 16; i++) {
      (*this)[i] = 0;
    }
  }


  void operator=(std::vector<int> const &rhs) {
    assert(rhs.size() == 16);
    for (int n = 0; n < 16; ++n) {
      (*this)[n] = rhs[n];
    }
  }


  Vec16 operator+(int rhs) const {
    Vec16 ret;

    for (int n = 0; n < 16; ++n) {
      ret[n] = (*this)[n] + rhs;
    }

    return ret;
  }


  Vec16 operator-(Vec16 const &rhs) const {
    Vec16 ret;

    for (int n = 0; n < 16; ++n) {
      ret[n] = (*this)[n] - rhs[n];
    }

    return ret;
  }


  Vec16 operator*(int rhs) const {
    Vec16 ret;

    for (int n = 0; n < 16; ++n) {
      ret[n] = (*this)[n]*rhs;
    }

    return ret;
  }


  Vec16 operator%(int rhs) const {
    Vec16 ret;

    for (int n = 0; n < 16; ++n) {
      ret[n] = (*this)[n] % rhs;
    }

    return ret;
  }


  bool operator==(Vec16 const &rhs) const {
    for (int n = 0; n < 16; ++n) {
      if((*this)[n] != rhs[n]) return false;
    }
    return true;
  }


  bool operator!=(Vec16 const &rhs) const { return !(*this == rhs); }


  bool is_uniform() const {
    for (int n = 1; n < 16; ++n) {
      if ((*this)[0] != (*this)[n]) {
        return false;
      }
    }

    return true;
  }


  int first() const {
    return (*this)[0];
  }


  std::string dump() const {
    std::string ret;

    for (int n = 0; n < (int) size(); ++n) {
      ret << (*this)[n] << ", ";
    }

    return ret;
  }


  static Vec16 index() {
    Vec16 ret;

    for (int n = 0; n < (int) ret.size(); ++n) {
      ret[n] = n;
    }

    return ret;
  }
};


Vec16 operator+(int lhs, Vec16 const &rhs) { return rhs + lhs; }


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
Offsets prepare_offsets(int s, int m, int log2n) {
  Offsets ret;

  int m2 = m >> 1;
  int n = 1 << log2n;

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
    ret << prepare_offsets(s, m, log2n);
  }

  return ret;
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


    // Load in vectors to handle
    for (int k = 0; k < k_count(); k++) {
      k_vec    << offsets[j + k].k_index;
      k_m2_vec << offsets[j + k].k_m2_index;
    }

    if (k_count() == 16) {
      m_step = 0;
    } else {
      m_step = k_vec[1] - k_vec[0];

#ifdef DEBUG
      int width = 16/k_count();
      assert(m_step > 0);
      assert(verify_step(k_vec,    m_step, width));
      assert(verify_step(k_m2_vec, m_step, width));
#endif  // DEBUG
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
  Vec16 k_16;

  Indexes16(CombinedOffsets const &co, int in_s, int in_j) {
    s = in_s;
    j = in_j;
    k_count     = co.k_count();
    step        = co.step();

    valid_index = co.valid_index(j);
  }


  Indexes16(CombinedOffsets const &co, int in_s, bool not_valid) {
    assert(not_valid == false);
    s = in_s;
    k_count     = co.k_count();
    valid_index = false;
  }


  /**
   * Constant offset between k and k_m2 vectors
   */
  int diff() const {
    return 1 << (s - 1);
  }


  Vec16 k_m2() const {
    return k_16 + diff();
  }


  bool same_indexes(Indexes16 const &rhs) const {
    return (s == rhs.s && j == rhs.j && k_count == rhs.k_count);
  }


  /**
   * @return uniform index offset if there is one, 0 otherwise
   */
  int index_offset(Indexes16 const &rhs) const {
    Vec16 k_tmp = k_16 - rhs.k_16;

    if (k_tmp.is_uniform()) {
      //std::cout << "uniform!\n";
      assert(k_tmp.first() != 0);
      return k_tmp.first();
    }

    //std::cout << "Not uniform: " << k_tmp.dump() << "\n";
    return 0;
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
              item.k_16[k]    = co.k_vec[offset + k];
            }

            ret << item;
          }
        } else {
          assert(co.k_vec.size() == 16);
          assert(co.k_m2_vec.size() == 16);

          Indexes16 item(co, s, j);
          item.k_16    = co.k_vec;

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


struct {
  int log2n = -1;
  std::vector<Offsets> fft_indexes;
  std::vector<Indexes16> vectors16;

  void init(int in_log2n) {
    if (log2n != in_log2n) {
      fft_indexes = prepare_fft_indexes(in_log2n);
      vectors16   = indexes_to_16vectors(fft_indexes);
    }

    log2n = in_log2n;

    // Check assumptions
    for (int i = 0; i < (int) vectors16.size(); i++) {
      auto &item  = vectors16[i];
      Vec16 k_diff = item.k_m2() - item.k_16;  // TODO cleanup
      assert(k_diff.is_uniform());

      if (item.k_count == 16) {
        assert((1 << (log2n - 1)) == k_diff.first());
      } else if (item.step != -1) {
        assert(item.step/2 == k_diff.first());
      }
/*
      if (item.step/2 != k_diff.first()) {
        std::cout << "i " << i << ": step " << item.step << " " << k_diff.dump() << std::endl;
      }
*/
    }
  }


  bool valid_index() const {
    assert(!vectors16.empty());
    return vectors16[0].valid_index;
  }


  /**
   * Count the number of consecutive items with the same s, j and k,
   * starting from index i
   */
  int same_indexes(int i) const {
    auto &item = vectors16[i];

    int same_count = 1;
    for (int j = i + 1; j < (int) vectors16.size(); j++) {
      auto &item2 = vectors16[j];
      if (item.same_indexes(item2)) {
        same_count++;
      } else {
        break;
      }
    }
/*
    if (same_count > 1) {  // True for log2n >= 6
      std::cout << "same_count: " << same_count << std::endl;
    }
*/
    return same_count;
  }


  /**
   * Count the number of consecutive items with the same index offset,
   * starting from index i
   *
   * Invalid index offsets are discounted
   */
  int same_index_offsets(int i) const {
    assert(i >= 0);

    if (i == 0) return 1;
    if (i >= (int) vectors16.size()) return 0;

    int same_count = 1;
    int prev_offset = vectors16[i].index_offset(vectors16[i - 1]);

    for (int j = i; j < (int) (vectors16.size() - 1); j++) {
      auto &item  = vectors16[j];
      auto &item2 = vectors16[j + 1];

      int index_offset = item2.index_offset(item);

      if (index_offset == 0 && prev_offset != index_offset) {
        std::cout << "prev_offset != index_offset" << std::endl;
      }

      if (index_offset != 0 && prev_offset == index_offset && item.same_indexes(item2)) {
        same_count++;
      } else {
        break;
      }
    }

    return same_count;
  }


  int same_index_count(int i, bool skip_j = false) const {
    assert(i >= 0);
    assert(i < (int) vectors16.size());

    int same_count = 1;
    auto &item  = vectors16[i];

    for (int j = i + 1; j < (int) vectors16.size(); j++) {
      auto &item2 = vectors16[j];
     
      if (skip_j) {
        bool same = (item.s == item2.s && item.k_count == item2.k_count);
        if (same) {
          same_count++;
        } else {
          break;
        }
      } else { 
        if (item.same_indexes(item2)) {
          same_count++;
        } else {
          break;
        }
      }
    }

    return same_count;
  }


  int m2_offset(int index) const {
    auto &item = vectors16[index];
    return (item.step == 0)? (1 << (log2n - 1)): item.step/2;
  }


  std::string dump() const {
    std::string ret;

    ret << "log2n: " << log2n << ", size: " << vectors16.size() <<  "\n";

    if (vectors16.empty()) {
      return ret;
    }

    //
    // Show counts
    //
    int max_s = -1;
    int max_j = -1;
    int max_k = -1;
    for (int i = 0; i < (int) vectors16.size(); i++) {
      auto &item = vectors16[i];
      if (max_s == -1 || max_s < item.s)       { max_s = item.s; }
      if (max_j == -1 || max_j < item.j)       { max_j = item.j; }
      if (max_k == -1 || max_k < item.k_count) { max_k = item.k_count; }
    }
    ret << "max_s : " << max_s << ", "
        << "max_j : " << max_j << ", "
        << "max_k : " << max_k << "\n";

    std::vector<std::vector<int>> counts;
    counts.resize(max_s + 1);

    for (int i = 0; i < (int) counts.size(); i++) {
      counts[i].resize(max_k + 1);
      for (int j = 0; j < (int) counts[i].size(); j++) {
        counts[i][j] = 0;
      }
    }

   
    for (int i = 0; i < (int) vectors16.size(); i++) {
      auto &item = vectors16[i];
      counts[item.s][item.k_count]++;
    }

    for (int i = 1; i < (int) counts.size(); i++) {  // Start with 1, there is no s == 0
      ret << "  s: " << i << "\n";
      for (int j = 0; j < (int) counts[i].size(); j++) {
        if (counts[i][j] != 0) {
          ret << "    k: " << j << ", " << counts[i][j] << " 16vecs\n";
        }
      }
    }

    return ret;
  }

} fft_context;



/**
 * Loop optimization: skip For-loop if only 1 iteration
 */
void Loop(int count, std::function<void(Int const &)> f) {
  assert(count > 0);
  if (count == 1) {
    f(0);
  } else {
    For (Int j = 0, j < count, j++)
      f(j);
    End
  }
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


/**
 * Do full initialization for offsets
 */
void init_k(Int &k_off, Indexes16 const &item) {
// Following can replace the entire content of this function
//  int mod = (1 << fft_context.log2n) - 1;
//  k_off = (item.k_16.first() + index()*(1 << item.s)) % mod;  // Issue: division in % is costly

  Vec16 const &k_in = item.k_16;
  int k_count       = item.k_count;

  int step    = item.step;
  if (k_count == 16) step = 1;

  k_off = index()*step + k_in.first();

  if (k_count == 1 || k_count == 16) return;

  int width = 16/k_count;
  for (int i = 1; i < k_count; i++) {
    Int offset2 = index() - width*i;

    Where (offset2 >= 0)
      k_off = offset2*step + k_in[i*width];
    End
  }
}


void fft_step(Complex::Ptr const &b_k, Complex::Ptr const &b_k_m2, Complex const &w) {
  Complex t;
  receive(t);
  t *= w;

  Complex u;
  receive(u);

  *b_k    = u + t;
  *b_k_m2 = u - t;
}


void fft_step(Complex::Ptr const &b_k, Complex const &w, int m2_offset) {
  Complex::Ptr b_k_m2 = b_k + m2_offset;
  fft_step(b_k, b_k_m2, w);
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
void tiny_fft(Complex::Ptr &b, Complex::Ptr &devnull) {
  debug("Fallback!");

  for (int i = 0; i < (int) fft_context.fft_indexes.size(); i++) {
    int s = i + 1;

    Complex w(1, 0);
    Complex wm(-1.0f/((float) (1 << s)));

    auto offsets = fft_context.fft_indexes[i];

    for (int j = 0; j < (int) offsets.size(); j++) {
      CombinedOffsets co(offsets, j);
      assert(!co.valid_index(j));  // Check if this is really for tiny fft

      auto &k_index    = offsets[j].k_index;
      auto &k_m2_index = offsets[j].k_m2_index;

      Complex::Ptr b_k_m2 = b;
      load_16ptr(k_m2_index, b_k_m2, devnull);
      gather(b_k_m2);

      Complex::Ptr b_k    = b;
      load_16ptr(k_index, b_k, devnull);
      gather(b_k);

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
 * This kernel works with `emu()` and on `v3d` with `call()`,
 * *not* with `interpret()` or on `vc4`. So be it. Life sucks sometimes.
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * Lesson learnt: Using loops is way more efficient than loop unroll (at least, for this kernel)
 *   Issues:
 *   - With full loop unroll, compile time + kernel size is (at least) O^2 with log2n;
 *     with jk-loops, it is linear (e.g. log2n = 17 compile time 1.5s, log2n = 7 1.1s).
 *   - Also, run time is way faster. Suspect that this is due to cache hits, otherwise optimization within
 *     QPUs for code locality.
 *
 *  Conclusion: don't be afraid of NOP-overhead of loops.
 *     
 */
void fft_kernel(Complex::Ptr b, Complex::Ptr devnull, Int::Ptr offsets) {
  b -= index();

  if (!fft_context.valid_index()) {  // this path taken for log2n <= 4
    tiny_fft(b, devnull);
    return;
  }

  int last_s        = -1;

  Complex w(0, 0);
  Complex wm(0, 0);
  Complex w_off;

  for (int i = 0; i < (int) fft_context.vectors16.size(); i++) {
    auto &item = fft_context.vectors16[i];

    assert(last_s != item.s);
    if (last_s != item.s) {
      w  = Complex(1, 0);
      wm = Complex(-1.0f/((float) (1 << item.s)));
      last_s = item.s;
    }

    assert(item.valid_index);

    w_off = w;
    init_mult_factor(w_off, wm, item.k_count);

    auto fetch = [&i] (Complex::Ptr const &b_k) {
      gather(b_k + fft_context.m2_offset(i));
      gather(b_k);
    };


    int same_count        = fft_context.same_index_count(i);
    int same_count_skipjs = fft_context.same_index_count(i, true);  // Always same per s for given log2n
    {
      std::string msg;
      msg << "s: " << item.s << ", j: " << item.j << ", k: " << item.k_count
          << ", step: " << item.step
          << ", same_count: " << same_count
          << ", same_count skipjs: " << same_count_skipjs;
      debug(msg);
    }

    assert(item.k_16.first() == 0);
    int j_count  = same_count_skipjs/same_count;
    int j_offset = item.k_count;
    int index_offset = fft_context.vectors16[i + 1].index_offset(item);

    Int k_init = 0;
    init_k(k_init, item);

    Loop(j_count, [&] (Int const &j) {
      Int k_off = k_init + j*j_offset;
      Int k_off_out = k_off;

      fetch(b + k_off);      // Prefetch
      k_off += index_offset;

      Loop(same_count, [&] (Int const & k) {
        fetch(b + k_off);
        fft_step(b + k_off_out, w_off, fft_context.m2_offset(i));

        k_off += index_offset;
        k_off_out += index_offset;
      });

      Complex dummy;   // Prefetch compensate extra fetch
      receive(dummy);
      receive(dummy);

      // Update w for the next round
      for (int i = 0; i < j_offset; i++) {
        w_off *= wm; 
      }
    });

    i += same_count_skipjs - 1;
  }
}


}  // anon namespace


TEST_CASE("FFT test with scalar [fft]") {
  cx a[] = { cx(0,0), cx(1,1), cx(3,3), cx(4,4), cx(4, 4), cx(3, 3), cx(1,1), cx(0,0) };

  //
  // Source: https://www.oreilly.com/library/view/c-cookbook/0596007612/ch11s18.html
  //
  SUBCASE("Scalar from example") {
    int const log2n = 3;
    int const Dim =  1 << log2n;
    cx b[Dim];

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

    fft(a, b, log2n);

    float precision = 5.0e-5f;
    for (int i = 0; i < Dim; ++i) {
      INFO("diff " << i << ": " << abs(b[i] - expected[i]));
      REQUIRE(abs(b[i] - expected[i]) < precision);
    }

    //scalar_dump(b, 8);
  }


  SUBCASE("Kernel derived from scalar") {
    if (!running_on_v3d()) return;  // FFT works only on v3d

    int const log2n = 3;
    int const Dim =  1 << log2n;

    cx scalar_result[Dim];
    fft(a, scalar_result, log2n);
    //scalar_dump(scalar_result, Dim);

    Complex::Array aa(16);
    aa.fill(V3DLib::complex(0.0f, 0.0f));
    Complex::Array result(16);
    Complex::Array devnull(16);
    Int::Array offsets;

    for (int i=0; i < Dim; ++i) {
      aa.re()[i] = (float) a[i].real();
      aa.im()[i] = (float) a[i].imag();
    }

    // Perform bit reversal outside of kernel
    for (int i = 0; i < Dim; ++i) {
      result[bitReverse(i, log2n)] = aa[i];
    }

    fft_context.init(log2n);
    auto k = compile(fft_kernel, V3D);
    //k.pretty(true, "fft_kernel.txt", false);
    k.load(&result, &devnull, &offsets);
    k.call();
    //std::cout << "Kernel output: " << result.dump() << std::endl;

    float precision = 5.0e-5f;
    for (int i = 0; i < Dim; ++i) {
      float diff = (float) abs(scalar_result[i] - result[i].to_complex());
      INFO("diff " << i << ": " << diff);
      REQUIRE(diff < precision);
    }
  }
}


TEST_CASE("FFT test with DFT [fft][test2]") {
  if (!running_on_v3d()) return;  // FFT works only on v3d

  auto init_result = [] (Complex::Array &result, Float::Array &a, int Dim, int log2n) {
    result.fill(V3DLib::complex(0.0f, 0.0f));

    // Perform bit reversal outside of kernel
    for (int i = 0; i < Dim; ++i) {
      result[bitReverse(i, log2n)] = complex(a[i], 0.0f);
    }
  };

  float precision = 0.0f;

  // The higher log2n, the more divergence of FFT results with scalar
  auto set_precision = [&precision] (int log2n) {
    if (log2n <= 8) {
      precision = 8.0e-5f;
    } else if (log2n <= 9) {
      precision = 5.0e-4f;
    } else if (log2n <= 11) {
      precision = 5.0e-3f;
    } else {  // Tested for log2n = 12 
      precision = 3e-2f;  // Pretty crappy precision here
    }
  };


  SUBCASE("Compare FFT and DFT output") {
    // FFT beats DFT     for >=  7
    // FFT beats scalar  for >=  8 
    // Precision fails   for >= 13 (really bad!)
    // Error             for >= 18 (heap not big enough)
    // Compile Seg fault for >= 32 (not enough memory)
    int log2n = 12;

    int Dim = 1 << log2n;
    set_precision(log2n);
    REQUIRE(precision > 0.0f);

    int size = Dim;
    if (size < 16) size = 16;

    Float::Array a(size);
    a.fill(0);
    for (int c = 0; c < Dim; ++c) {
      a[c] = wavelet_function(c, Dim);
    }
/*
    std::cout << "Input wavelet: ";
    for (int c = 0; c < Dim; ++c) {
      std::cout << a[c] << ", ";
    }
    std::cout << std::endl;
*/

    // Run scalar FFT for results comparison
    cx scalar_result[Dim];
    {
      cx a_scalar[Dim];
      for (int c = 0; c < Dim; ++c) {
        //a_scalar[c] = cx(wavelet_function(c, Dim), 0.0f); - This gives the wrong value!!!
        a_scalar[c] = cx(a[c], 0.0f);
      }

      Timer timer1("scalar FFT run time");
      fft(a_scalar, scalar_result, log2n);
      timer1.end();
      //scalar_dump(scalar_result, Dim);
    }

    // Run DFT for time comparison
    //
    // Calling interpret() or emu() actually works here (with warnings), but
    // the result, while plausible, diverges from the scalar results.
    // 
    // Of special note is that these calls return identical results. If there
    // is an error here, both interpreter and emulator have it in congruent form.
    //
    if (log2n <= 9) {  // Reg allocation fails above this
      Complex::Array2D result_dft;
      Timer timer1("DFT compile time");
      auto k = compile(kernels::dft_inline_decorator(a, result_dft), V3D);
      k.pretty(false, "obj/test/dft_compare_v3d.txt");
      timer1.end();
      std::cout << "DFT kernel size: " << k.v3d_kernel_size() << std::endl;
      std::cout << "combined " << compile_data.num_instructions_combined << " instructions" << std::endl;

      Timer timer2("DFT run time");
      k.load(&result_dft, &a);
      //k.setNumQPUs(1);
      k.call();
      timer2.end();

      //std::cout << "DFT result: " << result_dft.dump() << std::endl;

      INFO("comparing DFT with scalar");
      check_result1(scalar_result, result_dft, Dim, precision);
    }

    Complex::Array devnull(16);
    Int::Array offsets;
    Complex::Array result_inline(size);

    {
      init_result(result_inline, a, Dim, log2n);

      fft_context.init(log2n);

      Timer timer1("FFT inline compile time");
      auto k = compile(fft_kernel, V3D);
      k.pretty(false, "./obj/test/fft_inline_v3d.txt", true);  // segfault for log2n == 9
      k.dump_compile_data(false, "fft_inline_dump_v3d.txt");
      timer1.end();
      std::cout << "FFT inline kernel size: " << k.v3d_kernel_size() << std::endl;
      std::cout << "combined " << compile_data.num_instructions_combined << " instructions" << std::endl;

      Timer timer2("FFT inline run time");
      k.load(&result_inline, &devnull, &offsets);
      k.call();
      timer2.end();

      {
        std::string msg;
        msg << "Max diff: " << max_mag_diff(scalar_result, result_inline);
        debug(msg);
      }

      INFO("comparing FFT inline with scalar");
      check_result2(scalar_result, result_inline, Dim, precision);
    }

    // Plot output
    {
      float real_result[Dim];
      for (int c = 0; c < Dim; ++c) {
        real_result[c] = result_inline[c].to_complex().magnitude();
      }

      char const *file_prefix = "fft";
      std::string filename = "obj/test/";
      filename << file_prefix << "_result.pgm";

      PGM pgm(Dim, 100);
      pgm.plot(real_result, Dim)
         .save(filename.c_str());
    }
  }
}


std::set<int> &operator+=(std::set<int> &lhs, std::vector<int> const &rhs) {
  for (int i = 0; i < (int) rhs.size(); i++) {
    lhs.insert(rhs[i]);
  }

  return lhs;
}


TEST_CASE("FFT check offsets [fft][offsets]") {
  int log2n = 8;  // Following tests work for >= 5, tested till <= 22
  bool show = true;

  fft_context.init(log2n);
  std::cout << fft_context.dump() << std::endl;

  std::string ret;

  bool reset_s;
  bool reset_j;
  int s_count;
  int cur_s = -1;
  int cur_j = -1;
  int cur_k = -1;
  Vec16 cur_k_16;

  for (int i = 0; i < (int) fft_context.vectors16.size(); i++) {
    auto &item = fft_context.vectors16[i];

    reset_s = (cur_s != item.s);
    if (cur_s != item.s) {
      if (show) ret << "s: " << item.s << ", ";
      cur_s = item.s;
      cur_j = -1;
      cur_k = -1;
    }

    reset_j = reset_s || (cur_j != item.j);
    if (cur_j != item.j) {
      if (show)  ret << "j: " << item.j << ", ";
      cur_j = item.j;
      cur_k = -1;
    }

    if (reset_j) {
      s_count = 0;
    } else {
      s_count++;
    }


    if (cur_k != item.k_count) {
      if (show) ret << "k: " << item.k_count << ":\n";
      cur_k = item.k_count;
    }

    std::string cur;
    cur << "  " << item.k_16.dump() << "\n";

    INFO("s: " << item.s << ", j: " << item.j << ", k: " << item.k_count);
    INFO(cur);

    REQUIRE(cur_s == item.s);

    // Check horizontal offsets within vectors
    int mod = (1 << log2n) - 1;

    int hor_step = (1 << item.s);
    if (item.k_count == 16) hor_step = 1;
    //REQUIRE(hor_step == item.step);  // False for item.k_count == 16, where item.step == 0 (instead of 1)

    Vec16 rhs = (item.k_16.first() + Vec16::index()*hor_step) % mod;
    REQUIRE(item.k_16 == rhs);

    for (int k = 1; k < 16; ++ k) {
      REQUIRE(item.k_m2()[k] <= mod);
    }


    // Checks offsets between vectors in s-range
    if (reset_s) {
      REQUIRE(item.k_16[0] == 0);
      cur_k_16 = item.k_16;
    } else {
      int diff = 1 << (cur_s + 4);
      if (diff >= (1 << log2n)) {
        diff = 1 << (cur_s - 4);
      }

      REQUIRE(item.k_16[0] == cur_k_16[0] + s_count*diff + cur_j);
    }

    if (show) ret << cur;
  }

  if (show) std::cout << ret << std::endl;


  //
  // Check indexes unique within s-group.
  // This is interesting for doing multi-qpu in FFT.
  //
  cur_s = -1;
  s_count = 0;
  int s_num_indexes = -1;
  std::set<int> s_indexes;

  for (int i = 0; i < (int) fft_context.vectors16.size(); i++) {
    auto &item = fft_context.vectors16[i];

    if (cur_s != item.s) {
      if (s_count != 0) {
        INFO("cur_s: " << cur_s);
        REQUIRE((int) s_indexes.size() == s_num_indexes);
      }

      cur_s = item.s;
      s_count = 0;
      s_num_indexes = 0;
      s_indexes.clear();
    } else {
      s_indexes += item.k_16.parent();
      s_indexes += item.k_m2().parent();
      s_num_indexes += (int) item.k_16.size(); 
      s_num_indexes += (int) item.k_m2().size(); 

      s_count++;
    }
  }

  // Don't forget the final check!
  REQUIRE (s_count != 0);
  if (s_count != 0) {
    INFO("Final cur_s: " << cur_s);
    REQUIRE((int) s_indexes.size() == s_num_indexes);
  }
}


namespace {

std::vector<int> src_vec;

void vecload_kernel(Int::Ptr dst) {
  Int tmp = 0;
  load_16vec(tmp, src_vec);
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
    if (!running_on_v3d()) return;  // Setting non-consecutive ptr offsets works only on v3d

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
