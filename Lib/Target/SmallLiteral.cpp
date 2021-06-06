#include "SmallLiteral.h"
#include <stdio.h>
#include "Source/Expr.h"
#include "Support/basics.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

// Small literals are literals that fit in the small immediate field
// of the VideoCore-IV instruction set.

const int NUM_SMALL_FLOATS = 16;
const float smallFloats[NUM_SMALL_FLOATS] = {
    1.0
  , 2.0
  , 4.0
  , 8.0
  , 16.0
  , 32.0
  , 64.0
  , 128.0
  , 0.00390625
  , 0.0078125
  , 0.015625
  , 0.03125
  , 0.0625
  , 0.125
  , 0.25
  , 0.5
};


int encodeSmallInt(int val) {
  if (val >= 0 && val <= 15)
    return val;
  else if (val >= -16 && val <= -1)
    return 32 + val;

  return -1;
}


int encodeSmallFloat(float val) {
  if (val == 0.0)
    return 0;
  else {
    int index = -1;
    for (int i = 0; i < NUM_SMALL_FLOATS; i++)
      if (smallFloats[i] == val) {
        index = i;
        break;
      }

    if (index != -1)
      return 32 + index;
  }

  return -1;
}


/**
 * Encode a small literal according to Table 5 of the VideoCore-IV manual.
 * 
 * @return encoded lit value, -1 if expression cannot be encoded
 */
int encodeSmallLit(Expr const &e) {
  if (e.tag() == Expr::INT_LIT) { 
    return encodeSmallInt(e.intLit);
  } else if (e.tag() == Expr::FLOAT_LIT) {
    return encodeSmallFloat(e.floatLit);
  }

  return -1;
}


/**
 * Decode a small literal.
 */
Word decodeSmallLit(int x) {
  Word w;

  if (x >= 32) {
    w.floatVal = smallFloats[x - 32];
  } else if (x >= 16) {
    w.intVal = x - 32;
  } else {
    w.intVal = x;
  }

  return w;
}


std::string printSmallLit(int x) {
  std::string ret;

  if (x >= 32)
    ret << smallFloats[x - 32];
  else if (x >= 16)
    ret << (x - 32);
  else
    ret << x;

  return ret;
}

}  // namespace V3DLib
