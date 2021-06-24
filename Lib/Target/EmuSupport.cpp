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

Vec::Vec(int val) {
  for (int i = 0; i < NUM_LANES; i++)
    elems[i].intVal = val;
}


Vec::Vec(Imm imm) {
  switch (imm.tag()) {
    case Imm::IMM_INT32:
      for (int i = 0; i < NUM_LANES; i++)
        (*this)[i].intVal = imm.intVal();
      break;

    case Imm::IMM_FLOAT32:
      for (int i = 0; i < NUM_LANES; i++)
        (*this)[i].floatVal = imm.floatVal();
      break;

    case Imm::IMM_MASK:
      for (int i = 0; i < NUM_LANES; i++)
        (*this)[i].intVal = (imm.mask() >> i) & 1;
      break;

    default: assert(false); break;
  }
}


Vec::Vec(std::vector<int> const &rhs) {
  assert(rhs.size() == NUM_LANES);

  for (int i = 0; i < NUM_LANES; i++) {
    (*this)[i].intVal = rhs[i];
  }
}


Vec &Vec::operator=(int rhs) {
  for (int i = 0; i < NUM_LANES; i++) {
    (*this)[i].intVal = rhs;
  }

  return *this;
}


Vec &Vec::operator=(float rhs) {
  for (int i = 0; i < NUM_LANES; i++) {
    (*this)[i].floatVal = rhs;
  }

  return *this;
}


bool Vec::operator==(Vec const &rhs) const {
  for (int i = 0; i < NUM_LANES; i++) {
    if ((*this)[i].intVal != rhs[i].intVal) return false;
  }

  return true;
}


bool Vec::operator==(int rhs) const {
  breakpoint

  for (int i = 0; i < NUM_LANES; i++) {
    if ((*this)[i].intVal != rhs) return false;
  }

  return true;
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


Vec Vec::recip() const {
  Vec ret;

  for (int i = 0; i < NUM_LANES; i++) {
    float a = elems[i].floatVal;
    ret[i].floatVal = (a != 0)?1/a:0;      // TODO: not sure about value safeguard
  }

  return ret;
}


Vec Vec::recip_sqrt() const {
  Vec ret;

  for (int i = 0; i < NUM_LANES; i++) {
    float a = (float) ::sqrt(elems[i].floatVal);
    ret[i].floatVal = (a != 0)?1/a:0;      // TODO: not sure about value safeguard
  }

  return ret;
}


Vec Vec::exp() const {
  Vec ret;

  for (int i = 0; i < NUM_LANES; i++) {
    ret[i].floatVal = (float) ::exp2(elems[i].floatVal);
  }

  return ret;
}


Vec Vec::log() const {
  Vec ret;

  for (int i = 0; i < NUM_LANES; i++) {
    float a = (float) ::log2(elems[i].floatVal);  // TODO what would log2(0) return?
    ret[i].floatVal = a;
  }

  return ret;
}


bool Vec::apply(Op const &op, Vec a, Vec b) {
  bool handled = true;

  switch (op.op) {
    case RECIP    : *this = a.recip();      break;
    case RECIPSQRT: *this = a.recip_sqrt(); break;
    case EXP      : *this = a.exp();        break;
    case LOG      : *this = a.log();        break;
    default: handled = false;
   }

  if (handled) return true;
  return apply(ALUOp(op), a, b);
}


bool Vec::apply(ALUOp const &op, Vec a, Vec b) {
  bool handled = true;
  if (op.value() == ALUOp::NOP) return true;

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

  assertq(handled, "Vec::apply(): Unhandled op value");
  return handled;
}


/**
 * Check if all vector elements of current have the same value
 *
 * @return true if all elements same value, false otherwise
 */
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


///////////////////////////////////////////////////////////////////////////////
// Class EmuState
///////////////////////////////////////////////////////////////////////////////

Vec const EmuState::index_vec({0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});

EmuState::EmuState(int in_num_qpus, IntList const &in_uniforms, bool add_dummy) :
  num_qpus(in_num_qpus),
  uniforms(in_uniforms)
{
  // Initialise semaphores
  for (int i = 0; i < 16; i++) sema[i] = 0;

  if (add_dummy) {
    // Add final dummy uniform for emulator
    // See Note 1, function `invoke()` in `vc4/Invoke.cpp`.
    uniforms << 0;
  }
}


Vec EmuState::get_uniform(int id, int &next_uniform) {
  Vec a;

  assert(next_uniform < uniforms.size());
  if (next_uniform == -2)
    a = id;
  else if (next_uniform == -1)
    a = num_qpus;
  else
    a = uniforms[next_uniform];

  next_uniform++;
  return a;
}


/**
 * Increment semaphore
 */
bool EmuState::sema_inc(int sema_id) {
  assert(sema_id >= 0 && sema_id < 16);
  if (sema[sema_id] == 15) {
    semaphore_wait_count++;
    assertq(semaphore_wait_count < MAX_SEMAPHORE_WAIT, "Semaphore wait for SINC appears to be stuck");
    return true;
  } else {
    semaphore_wait_count = 0;
    sema[sema_id]++;
    return false;
  }
}


/**
 * Decrement semaphore
 */
bool EmuState::sema_dec(int sema_id) {
  assert(sema_id >= 0 && sema_id < 16);
  if (sema[sema_id] == 0) {
    semaphore_wait_count++;
    assertq(semaphore_wait_count < MAX_SEMAPHORE_WAIT, "Semaphore wait for SDEC appears to be stuck");
    return true;
  } else {
    semaphore_wait_count = 0;
    sema[sema_id]--;
    return false;
  }
}

}  // namespace V3DLib
