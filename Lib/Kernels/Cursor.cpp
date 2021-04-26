#include "Cursor.h"
#include "Source/Lang.h"
#include "Source/gather.h"


namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class Cursor::Block
///////////////////////////////////////////////////////////////////////////////

Cursor::Block::Block(Cursor const &c, int in_first_index) : m_cursor(c), first_index(in_first_index) {
  for (int i = 0; i < 3; i++) {
    c.row[first_index - 1 + i].shiftLeft(m_right[i]);
    c.row[first_index - 1 + i].shiftRight(m_left[i]);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Class Cursor::Line
///////////////////////////////////////////////////////////////////////////////

void Cursor::Line::init(Float::Ptr in_src) {
  gather(in_src); comment("Cursor init");
  current = 0.0f;
  src = in_src + 16;
}


void Cursor::Line::init(Float::Ptr in_src, Float::Ptr const &in_dst) {
  init(in_src);
  has_dst = true;
  dst = in_dst;
}


void Cursor::Line::prime() {
  receive(next);
  gather(src);
}


/**
 * Advance the cursor line by one vector, i.e. slide the window right by one vector
 */
void Cursor::Line::advance() {
  src.inc();     comment("Cursor advance");
  prev = current;
  gather(src);
  current = next;
  receive(next);
}


void Cursor::Line::finish() {
  receive(next);
}


/**
 * Shift-left the current vector by one element,
 * using the value of the `next` vector
 *
 * The operation `rotate(x, n)` will rotate 16-vector
 *`x` right by `n` places where `n` is a integer in the range 0 to 15.
 * Rotating right by 15 is the same as rotating left by 1.
 */
void Cursor::Line::shiftLeft(Float& result) const {
  result = rotate(current, 15); comment("Cursor shiftLeft");
  Float nextRot = rotate(next, 15);
  Where (index() == 15)
    result = nextRot;
  End
}


/**
 * Shift-right the current vector by one element,
 * using the value of the prev vector.
 */
void Cursor::Line::shiftRight(Float& result) const {
  result = rotate(current, 1); comment("Cursor shiftRight");
  Float prevRot = rotate(prev, 1);
  Where (index() == 0)
    result = prevRot;
  End
}


///////////////////////////////////////////////////////////////////////////////
// Class Cursor
///////////////////////////////////////////////////////////////////////////////

Cursor::Cursor(Int const &width, int numlines) : m_num_lines(numlines), m_width(width) {
  assert(numlines >= 3);
  row.resize(m_num_lines);
}

/**
 * Initialize the cursor lines for the input and output rows
 *
 * Currently, only 3 lines are used.
 *
 * The outer rows do not output anything and don't need a dst vector.
 */
void Cursor::init(Float::Ptr const &in_src, Float::Ptr const &in_dst) {
  int i = 0;
  row[i].init(in_src + (i - 1)*m_width);
  for (i = 1; i < m_num_lines - 1; i++) row[i].init(in_src + (i - 1)*m_width, in_dst + (i - 1)*m_width);
  row[i].init(in_src + (i - 1)*m_width);

  for (int i = 0; i < m_num_lines; i++) row[i].prime();
}


void Cursor::step(std::function<void(Block const &, Float &)> f) {
  advance();

  for (int i = 1; i < m_num_lines - 1; ++i) {
    assert(row[i].has_dst);
    Block b(*this, i);

    Float output = 0.0f;
    f(b, output);

    *row[i].dst = output;
    row[i].dst.inc();
  }
}


void Cursor::advance() {
  for (int i = 0; i < m_num_lines; i++) row[i].advance();
}


void Cursor::finish() {
  for (int i = 0; i < m_num_lines; i++) row[i].finish();
}

}  // namespace V3DLib
