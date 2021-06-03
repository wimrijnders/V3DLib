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
  //using Parent::operator<<;
  using Parent::operator[];
  using Parent::size;

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

  Vec16 operator-(Vec16 const &rhs) const {
    Vec16 ret;

    for (int n = 0; n < 16; ++n) {
      ret[n] = (*this)[n] - rhs[n];
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

  Parent const &parent() const { return *this; }
};


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
  Vec16 k_m2_16;

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


  bool same_indexes(Indexes16 const &rhs) const {
    return (s == rhs.s && j == rhs.j && k_count == rhs.k_count);
  }
/*
  bool same_step(Indexes16 const &rhs) const {
    return (step == rhs.step && k_count == rhs.k_count);
  }
*/


  /**
   * @return uniform index offset if there is one, 0 otherwise
   */
  int index_offset(Indexes16 const &rhs) const {
    Vec16 k_tmp    = k_16    - rhs.k_16;
    Vec16 k_m2_tmp = k_m2_16 - rhs.k_m2_16;

    if (k_tmp.is_uniform() && k_m2_tmp.is_uniform()) {
      //std::cout << "uniform!\n";
      assert(k_tmp.first() == k_m2_tmp.first());
      assert(k_tmp.first() != 0);
      return k_tmp.first();
    }

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
              item.k_m2_16[k] = co.k_m2_vec[offset + k];
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


struct {
  int log2n = -1;
  std::vector<Offsets> fft_indexes;
  std::vector<Indexes16> vectors16;
  bool use_offsets = false;

  int const k_limit = 1;

  void init(int in_log2n, bool in_use_offsets) {
    if (log2n != in_log2n) {
      fft_indexes = prepare_fft_indexes(in_log2n);
      vectors16   = indexes_to_16vectors(fft_indexes);
    }

    log2n = in_log2n;
    use_offsets = in_use_offsets;

    // Check assumptions
    for (int i = 0; i < (int) vectors16.size(); i++) {
      auto &item  = vectors16[i];
      Vec16 k_diff = item.k_m2_16 - item.k_16;
      assert(k_diff.is_uniform());

      if (item.step == 0) {
        assert((1 << (log2n - 1)) == k_diff.first());
      } else {
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
   * For Dim <=4, this will not init the offsets array, because the indexes are invalid.
   * In the kernel, this means that the offset buffer is not used.
   */
  void init_offsets_array(Int::Array &offsets) const {
    if (!valid_index()) return;

    offsets.alloc((uint32_t) (16*offsets_size()));
    int dst_index = 0;

    for (int i = 0; i < (int) vectors16.size(); i++) {
      auto &vec = vectors16[i];
      if (vec.k_count < k_limit) continue;
      assert(vec.k_16.size() == 16);

      for (int j = 0; j < 16; j++) {
        offsets[dst_index] = vec.k_16[j];
        dst_index++;
      }
    }
  }


  int offsets_size() const {
    int count = 0;

    for (int i = 0; i < (int) vectors16.size(); i++) {
      auto &item = vectors16[i];
      if (item.k_count >= k_limit) count++;     
    }

    assert(count > 0);
    return count;
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
    if (i <= 0) {
      return 1;
    } 
    if (i >= (int) vectors16.size()) {
      return 1;
    } 

    int same_count = 1;
    int prev_offset = vectors16[i].index_offset(vectors16[i - 1]);

    for (int j = i + 1; j < (int) vectors16.size(); j++) {
      auto &item  = vectors16[j - 1];
      auto &item2 = vectors16[j];

      int index_offset = item2.index_offset(item);
      if (index_offset != 0 && prev_offset == index_offset && item.same_indexes(item2)) {
        same_count++;
      } else {
        break;
      }
    }
/*
    if (same_count > 1) {  // True for log2n >= 6
      std::cout << "same_count: " << me_count << std::endl;
    }
*/

/*
    // Check assumptions

    if (same_count > 1) {  // True for log2n >= 6

    Vec16 k_base_diff = vectors16[i].k_m2_16 - vectors16[i].k_16;
    bool checks_out = true;
    for (int c = 1; c < same_count; c++) {
      auto &item  = vectors16[i + c];
      Vec16 k_diff = item.k_m2_16 - item.k_16;
      if (k_diff != k_base_diff) {
        checks_out = false;
        break;
      }
    }
    assert(checks_out);
    assert(k_base_diff.is_uniform());
    std::cout << "step " << vectors16[i].step << ": " << k_base_diff.dump() << std::endl;

    }
*/

    return same_count;
  }


  std::string dump() const {
    std::string ret;

    ret << "log2n: " << log2n << ", size: " << vectors16.size() <<  "\n";

    if (vectors16.empty()) {
      return ret;
    }

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


int m2_offset(Indexes16 const &item) {
  return (item.step == 0)? (1 << (fft_context.log2n - 1)): item.step/2;
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
 * Do full init test for offsets
 */
void init_k(Int &k_off, Indexes16 const &item) {
  Vec16 const &k_in    = item.k_16;

  // Do the full inits
  int step    = item.step;
  int k_count = item.k_count;

  while (true) {

  if (k_count == 16) {
    // There is no step; load values in directly
    load_16vec(k_off, k_in.parent());
    break;;
  }

  assert(step != -1);
  k_off     = k_in[0];
  k_off    += index()*step;

  if (k_count == 1) {
    break;
  }

  int width = 16/k_count;
  for (int i = 1; i < k_count; i++) {
    Int offset2 = index() - width*i;

    Where (offset2 >= 0)
      k_off     = k_in[i*width];
      k_off    += offset2*step;
    End
  }

    break;
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
  //debug("Fallback!");

  for (int i = 0; i < (int) fft_context.fft_indexes.size(); i++) {
    int s = i + 1;

    Complex w(1, 0);
    Complex wm(-1.0f/((float) (1 << s)));

    auto offsets = fft_context.fft_indexes[i];

    for (int j = 0; j < (int) offsets.size(); j++) {
      Complex w_off;

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
 * Update w for the next round
 */
void fft_loop_increment(Complex &w, Complex const &wm, int i) {
  if (i + 1 >= (int) fft_context.vectors16.size()) {  // Don't bother doing this for the last item
    return;
  }

  auto &item = fft_context.vectors16[i];
  auto &next_item = fft_context.vectors16[i + 1];

  if (item.j != next_item.j) {
    // Extra loop offset for combined rows
    for (int k = 1; k < item.k_count; k++) {
      w *= wm; 
    }

    w *= wm;
  }
}


void fft_calc(Complex::Ptr const &b, Int const &k_off, Complex &w_off, int m2_offset) {
  Complex::Ptr b_k = b + k_off;
  gather(b_k + m2_offset);
  gather(b_k);

  fft_step(b_k, w_off, m2_offset);
}


/**
 * Kernel derived from scalar
 *
 * The result is returned in b.
 *
 * This kernel works with `emu()` and on `v3d` with `call()`, *not* with `interpret()` or on `vc4`.
 * So be it. Life sucks sometimes.
 */
void fft_kernel(Complex::Ptr b, Complex::Ptr devnull, Int::Ptr offsets) {
  b -= index();

  if (!fft_context.valid_index()) {  // this path taken for log2n <= 4
    tiny_fft(b, devnull);
    return;
  }

  int last_s = -1;
  Complex w(0, 0);
  Complex wm(0, 0);

  int last_k_count  = -1;
  int last_j        = -1;
  Complex w_off;

  Int k_off     = 0;

  for (int i = 0; i < (int) fft_context.vectors16.size(); i++) {
    auto &item = fft_context.vectors16[i];

    if (last_s != item.s) {
      w = Complex(1, 0);
      wm = Complex(-1.0f/((float) (1 << item.s)));
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

    bool do_inline = !(fft_context.use_offsets && item.k_count >= fft_context.k_limit);
    if (do_inline) {
      int index_offset = 0;

      if (i > 0) {
       index_offset = fft_context.vectors16[i].index_offset(fft_context.vectors16[i - 1]);
      }

      auto &item = fft_context.vectors16[i];
      int same_count = fft_context.same_index_offsets(i);

      if (index_offset == 0) {
        init_k(k_off, fft_context.vectors16[i]);
        fft_calc(b, k_off, w_off, m2_offset(item));
      } else if (same_count == 1) {
          k_off    += index_offset;
          fft_calc(b, k_off, w_off, m2_offset(item));
      } else {
        assert(same_count % 2 == 1);

          auto fetch = [&item] (Complex::Ptr const &b_k) {
            gather(b_k + m2_offset(item));
            gather(b_k);
          };

          Int k_off_out = k_off;

           // k_off += index_offset;
           // fetch(b + k_off);

          For (Int j = 0, j < same_count, j++)
            k_off += index_offset;
            fetch(b + k_off);

            k_off_out += index_offset;
            fft_step(b + k_off_out, w_off, m2_offset(item));
          End

          //Complex dummy;
          //receive(dummy);
          //receive(dummy);

          i += same_count - 1;
      }
    } else {
      int same_count = fft_context.same_indexes(i);
      if (same_count == 1) {
        gather(offsets);
        offsets.inc();
        receive(k_off);
        fft_calc(b, k_off, w_off, m2_offset(item));
      } else {
        For (Int j = 0, j < same_count, j++)
          gather(offsets);
          offsets.inc();
          receive(k_off);

          fft_calc(b, k_off, w_off, m2_offset(item));
        End
        i += same_count - 1;
      }
    }

    fft_loop_increment(w, wm, i);
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

    fft_context.init(log2n, false);
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
    int log2n = 7;  // Tested up till 12 (compile times FFT buffer: 119s, inline: 56s)
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


    // FFT inline offsets
    /* if (log2n <= 10) */ {  // After this segfault
      Complex::Array result_inline(size);
      init_result(result_inline, a, Dim, log2n);

      fft_context.init(log2n, false);

      Timer timer1("FFT inline compile time");
      auto k = compile(fft_kernel, V3D);
      //k.pretty(false, "fft_inline_v3d.txt", false);  // segfault for log2n == 9
      //k.dump_compile_data(false, "fft_inline_dump_v3d.txt");
      timer1.end();
      std::cout << "FFT inline kernel size: " << k.v3d_kernel_size() << std::endl;
      std::cout << "combined " << compile_data.num_instructions_combined << " instructions" << std::endl;

      Timer timer2("FFT inline run time");
      k.load(&result_inline, &devnull, &offsets);
      k.call();
      timer2.end();

      //std::cout << "FFT result: " << result.dump() << std::endl;
      INFO("comparing FFT inline with scalar");
      check_result2(scalar_result, result_inline, Dim, precision);
    }


    // FFT offsets in buffer
    Complex::Array result_buf(size);
    {
      init_result(result_buf, a, Dim, log2n);

      fft_context.init(log2n, true);
      //std::cout << fft_context.dump() << std::endl;
      fft_context.init_offsets_array(offsets);

      Timer timer1("FFT buffer compile time");
      auto k = compile(fft_kernel, V3D);
      k.dump_compile_data(false, "fft_buffer_data.txt");
      timer1.end();
      std::cout << "FFT buffer kernel size: " << k.v3d_kernel_size() << std::endl;
      std::cout << "combined " << compile_data.num_instructions_combined << " instructions" << std::endl;

      Timer timer2("FFT buffer run time");
      k.load(&result_buf, &devnull, &offsets);
      k.call();
      timer2.end();

      INFO("comparing FFT buffer with scalar");
      check_result2(scalar_result, result_buf, Dim, precision);
    }

    // output plot
    {
      float real_result[Dim];
      for (int c = 0; c < Dim; ++c) {
        real_result[c] = result_buf[c].to_complex().magnitude();
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
