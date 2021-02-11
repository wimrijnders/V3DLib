#include "EmuSupport.h"
#include <cmath>
#include <cstdio>
#include <cstring>  // strlen()
#include "Support/basics.h"

namespace V3DLib {
namespace {

// Bitwise rotate-right
inline int32_t rotRight(int32_t x, int32_t n) {
  uint32_t ux = (uint32_t) x;
  return (ux >> n) | (x << (32-n));
}

}  // anon namespace

// ============================================================================
// Rotate a vector
// ============================================================================

Vec rotate(Vec v, int n) {
  Vec w;
  for (int i = 0; i < NUM_LANES; i++)
    w[(i+n) % NUM_LANES] = v[i];
  return w;
}


// ============================================================================
// Printing routines
// ============================================================================

namespace {

void emitChar(Seq<char>* out, char c) {
  if (out == nullptr) printf("%c", c);
  else *out << c;
}

}  // anon namespace


void emitStr(Seq<char>* out, const char* s) {
  if (out == nullptr)
    printf("%s", s);
  else
    for (int i = 0; i < (int) strlen(s); i++)
      *out << s[i];
}


void printIntVec(Seq<char>* out, Vec x) {
  char buffer[1024];
  emitChar(out, '<');
  for (int i = 0; i < NUM_LANES; i++) {
    snprintf(buffer, sizeof(buffer), "%i", x[i].intVal);

    for (int j = 0; j < (int) strlen(buffer); j++) emitChar(out, buffer[j]);
    if (i != NUM_LANES-1) emitChar(out, ',');
  }
  emitChar(out, '>');
}



void printFloatVec(Seq<char>* out, Vec x) {
  char buffer[1024];
  emitChar(out, '<');
  for (int i = 0; i < NUM_LANES; i++) {
    snprintf(buffer, sizeof(buffer), "%f", x[i].floatVal);

    for (int j = 0; j < (int) strlen(buffer); j++) emitChar(out, buffer[j]);
    if (i != NUM_LANES-1) emitChar(out, ',');
  }
  emitChar(out, '>');
}


// ============================================================================
// Class Vec
// ============================================================================

Vec Vec::Always(1);

Vec::Vec(int val) {
  for (int i = 0; i < NUM_LANES; i++)
    elems[i].intVal = 1;
}


std::string Vec::dump() const {
  std::string ret;
  ret << "<";

  for (int i = 0; i < NUM_LANES; i++) {
    ret << elems[i].intVal << ", ";
  }

  ret << ">";
  return ret;
}


/**
 * Negate a condition vector
 */
Vec Vec::negate() const {
  Vec v;

  for (int i = 0; i < NUM_LANES; i++)
    v[i].intVal = !elems[i].intVal;
  return v;
}


bool Vec::apply(Op op, Vec a, Vec b) {
  bool did_something = true;

  if (op.type == FLOAT) {
    // Floating-point operation
    for (int i = 0; i < NUM_LANES; i++) {
      float  x = a[i].floatVal;
      float  y = b[i].floatVal;
      float &d = elems[i].floatVal;

      switch (op.op) {
        case ADD      : d = x+y;                   break;
        case SUB      : d = x-y;                   break;
        case MUL      : d = x*y;                   break;
        case ItoF     : d = (float) a[i].intVal;   break;
        case FtoI     : elems[i].intVal = (int) x; break;
        case MIN      : d = x<y?x:y;               break;
        case MAX      : d = x>y?x:y;               break;
        case RECIP    : d = 1/x;                   break; // TODO guard against zero?
        case RECIPSQRT: d = (float) (1/::sqrt(x)); break; // TODO idem
        case EXP      : d = (float) ::exp2(x);     break;
        case LOG      : d = (float) ::log2(x);     break; // TODO idem
        default: assert(false);
      }
    }
  } else if (op.type == INT32) {
    // Integer operation
    for (int i = 0; i < NUM_LANES; i++) {
      int32_t x   = a[i].intVal;
      int32_t y   = b[i].intVal;
      int32_t &d  = elems[i].intVal;
      uint32_t ux = (uint32_t) x;

      switch (op.op) {
        case ADD:  d = x+y; break;
        case SUB:  d = x-y; break;
        case MUL:  d = (x&0xffffff)*(y&0xffffff); break;
        case SHL:  d = x<<y; break;
        case SHR:  d = x>>y; break;
        case USHR: d = (int32_t) (ux >> y); break;
        case ItoF: elems[i].floatVal = (float) a[i].intVal; break;
        case FtoI: d = (int) a[i].floatVal; break;
        case MIN:  d = x<y?x:y; break;
        case MAX:  d = x>y?x:y; break;
        case BOR:  d = x|y; break;
        case BAND: d = x&y; break;
        case BXOR: d = x^y; break;
        case BNOT: d = ~x; break;
        case ROR:  d = rotRight(x, y);

        default: assert(false);
      }
    }
  } else {
    did_something = false;
  }

  return did_something;
}


bool Vec::is_uniform() const {
  int val = elems[0].intVal;

  for (int i = 1; i < NUM_LANES; i++) {
    if (elems[i].intVal != val) return false;
  }

  return true;
}


}  // namespace V3DLib
