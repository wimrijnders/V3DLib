#include "EmuSupport.h"
#include <cmath>
#include <cstdio>
#include <cstring>  // strlen()
#include "Support/basics.h"
#include "Target/instr/ALUOp.h"
#include "Source/Op.h"

namespace V3DLib {
namespace {

// Bitwise rotate-right
inline int32_t rotRight(int32_t x, int32_t n) {
  uint32_t ux = (uint32_t) x;
  return (ux >> n) | (x << (32-n));
}


// Count leading zeros
inline int32_t clz(int32_t x) {
  int32_t count = 0;
  int32_t n = (int32_t) (sizeof(int)*8);
  for (int32_t i = 0; i < n; i++) {
    if (x & (1 << (n-1))) break;
    else count++;
    x <<= 1;
  }

  return count;
}


/**
 * Rotate a vector
 */
Vec rotate(Vec v, int n) {
  Vec w;
  for (int i = 0; i < NUM_LANES; i++)
    w[(i+n) % NUM_LANES] = v[i];
  return w;
}

}  // anon namespace


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


bool Vec::apply(Op const &op, Vec a, Vec b) {
  bool handled = true;

  for (int i = 0; i < NUM_LANES; i++) {
    float  x = a[i].floatVal;
    float &d = elems[i].floatVal;

    switch (op.op) {
      case RECIP    : d = 1/x;                   break; // TODO guard against zero?
      case RECIPSQRT: d = (float) (1/::sqrt(x)); break; // TODO idem
      case EXP      : d = (float) ::exp2(x);     break;
      case LOG      : d = (float) ::log2(x);     break; // TODO idem
      default: handled = false;;
    }
  }

  if (handled) return true;

  return apply(ALUOp(op), a, b);
}


bool Vec::apply(ALUOp const &op, Vec a, Vec b) {
  bool handled = true;

  // Floating-point operations
  for (int i = 0; i < NUM_LANES; i++) {
    float  x = a[i].floatVal;
    float  y = b[i].floatVal;
    float &d = elems[i].floatVal;

    switch (op.value()) {
    case ALUOp::A_FADD:    d = x+y;                       break;
    case ALUOp::A_FSUB:    d = x-y;                       break;
    case ALUOp::A_FMIN:    d = x<y?x:y;                   break;
    case ALUOp::A_FMAX:    d = x>y?x:y;                   break;
    case ALUOp::A_FMINABS: d = fabs(x) < fabs(y) ? x : y; break; // min of absolute values
    case ALUOp::A_FMAXABS: d = fabs(x) > fabs(y) ? x : y; break; // max of absolute values
    case ALUOp::A_FtoI:    elems[i].intVal = (int) x;     break;
    case ALUOp::A_ItoF:    d = (float) a[i].intVal;       break;
    case ALUOp::M_FMUL:    d = x*y;                       break;

    default:
      handled = false;
      break;
    }
  }

  if (handled) return handled;
  handled = true;

  // Integer operations
  for (int i = 0; i < NUM_LANES; i++) {
    int  x = a[i].intVal;
    int  y = b[i].intVal;
    int &d = elems[i].intVal;

    switch (op.value()) {
    case ALUOp::A_ADD:   d = x+y;            break;
    case ALUOp::A_SUB:   d = x-y;            break;
    case ALUOp::A_ROR:   d = rotRight(x, y); break;
    case ALUOp::A_SHL:   d = x<<y;           break;
    case ALUOp::A_SHR:   d = (int32_t) (((uint32_t) x) >> y); break;
    case ALUOp::A_ASR:   d = x >> y; break;
    case ALUOp::A_MIN:   d = x<y?x:y;        break;
    case ALUOp::A_MAX:   d = x>y?x:y;        break;
    case ALUOp::A_BAND:  d = x&y;            break;
    case ALUOp::A_BOR:   d = x|y;            break;
    case ALUOp::A_BXOR:  d = x^y;            break;
    case ALUOp::A_BNOT:  d = ~x; break;
    case ALUOp::M_MUL24: d = (x&0xffffff)*(y&0xffffff); break; // Integer multiply (24-bit)

    case ALUOp::A_CLZ:    d = clz(x);         break; // Count leading zeros

    case ALUOp::A_V8ADDS:
    case ALUOp::A_V8SUBS:
    case ALUOp::M_V8MUL:
    case ALUOp::M_V8MIN:
    case ALUOp::M_V8MAX:
    case ALUOp::M_V8ADDS:
    case ALUOp::M_V8SUBS: {
      std::string buf;
      buf << "V3DLib: unsupported operator " << op.value();
      assertq(false, buf);
    }
    break;

    default:
      handled = false;
      break;
    }
  }

  if (handled) return handled;
  handled = true;

  // Other operations
  switch (op.value()) {
    case ALUOp::M_ROTATE: { // Vector rotation
      assert(b.is_uniform());
      int n = b[0].intVal;

      assign(rotate(a,n));
    }
    break;

    default:
      handled = false;
    break;
  }

  return handled;
}


bool Vec::is_uniform() const {
  int val = elems[0].intVal;

  for (int i = 1; i < NUM_LANES; i++) {
    if (elems[i].intVal != val) return false;
  }

  return true;
}


void Vec::assign(Vec const &rhs) {
  for (int i = 0; i < NUM_LANES; i++) {
    elems[i] = rhs[i];
  }
}


}  // namespace V3DLib
