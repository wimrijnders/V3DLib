#ifndef _V3DLIB_KERNELS_CURSOR_H_
#define _V3DLIB_KERNELS_CURSOR_H_
#include <functional>
#include "Source/Float.h"

namespace V3DLib {

/**
 * Handle multiple lines of input data.
 *
 * (Pre)loads values for lines and iterates over them.
 *
 * Extracted from example HeatMap, because it has more potential uses.
 * A good usage would be for cellular automata.
 *
 * This implementation uses Float values. It can be expanded to also work
 * with Int values, when I find a decent reason to use them.
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * A possible extension is to have the edges wraparouns for a torus topology.
 *   Will implement this when I need it.
 *
 * * The class has been set up to use multiple lines, i.e. > 3. The idea was
 *   to make the line loading more efficient.
 *   In practise, it turns out that this gives absolutely no performance benefit;
 *   in addition, compile times shoot up.
 *   It looks like 3 lines is the optimum for loading.
 *
 * * Effect NumLines > 3 fot HeatMap: 
 *   20210424:
 * 
 *   - Works okay for <= 8, with small differences in output
 *   -  9, 10 breaks down - white screen
 *   -  11: register allocation failed, insufficient capacity
 * 
 *   The insult here is that the original 3 still appears to work best.
 *
 */
class Cursor {
public:
  /**
   * Represent a 3x3 block of values which are currently active
   */
  struct Block {
    Block(Cursor const &c, int in_first_index);

    Float const &left(int n)    const  { return m_left[n]; }
    Float const &current(int n) const  { return m_cursor.row[first_index - 1 + n].current; }
    Float const &right(int n)   const  { return m_right[n]; }

  private:
    Cursor const &m_cursor;
    int    first_index;
    Float  m_left[3];
    Float  m_right[3];
  };


  Cursor(Int const &width, int numlines = 3);

  int offset() const { return m_num_lines -2; }
  void init(Float::Ptr const &in_src, Float::Ptr const &in_dst);
  void step(std::function<void(Block const &, Float &)> f);
  void finish();

private:

  struct Line {
    void init(Float::Ptr in_src);
    void init(Float::Ptr in_src, Float::Ptr const &in_dst);

    void prime();
    void advance();
    void finish();
    void shiftLeft(Float& result) const;
    void shiftRight(Float& result) const;

    Float::Ptr src;
    Float::Ptr dst;
    bool has_dst = false;
    Float prev, current, next;
  };

  void advance();

  std::vector<Line> row;
  int m_num_lines = 3;
  Int m_width;
};

}  // namespace V3DLib

#endif  // _V3DLIB_KERNELS_CURSOR_H_
