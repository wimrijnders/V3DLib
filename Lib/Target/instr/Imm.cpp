#include "Imm.h"
#include "Support/basics.h"
#include "Support/Platform.h"
#include "v3d/instr/SmallImm.h"

namespace V3DLib {

Imm::Imm(int i)   : m_tag(IMM_INT32),   m_intVal(i) {}
Imm::Imm(float f) : m_tag(IMM_FLOAT32), m_floatVal(f) {}

int   Imm::intVal()   const { assert(m_tag == IMM_INT32);   return m_intVal; }
int   Imm::mask()     const { assert(m_tag == IMM_MASK);    return m_intVal; }
float Imm::floatVal() const { assert(m_tag == IMM_FLOAT32); return m_floatVal; }

bool Imm::is_zero() const { return m_tag == IMM_INT32 && m_intVal == 0; }


/**
 * Check if this is an immediate value which does not
 * need to be constructed inline.
 */
bool Imm::is_basic() const {
  if (Platform::compiling_for_vc4()) {
    return true;  // All values are basic for vc4
  }

  assert(m_tag != IMM_MASK);  // Not dealing with this here
  int dummy;

  if (is_int()) {
    if (v3d::instr::SmallImm::int_to_opcode_value(m_intVal, dummy)) {
      return true;
    }
  }

  if (is_float()) {
    if (v3d::instr::SmallImm::float_to_opcode_value(m_floatVal, dummy)) {
      return true;
    }
  }

  return false;
}


std::string Imm::pretty() const {
  std::string ret;

  switch (m_tag) {
    case IMM_INT32:   ret << m_intVal;   break;
    case IMM_FLOAT32: ret << m_floatVal; break;

    case IMM_MASK: {
        int b = m_intVal;

        for (int i = 0; i < 16; i++) {
          ret << ((b & 1)? 1 : 0);
          b >>= 1;
        }
    }
    break;

    default: assert(false); break;
  }

  return ret;
}


bool Imm::operator==(Imm const &rhs) const {
  if (m_tag != rhs.m_tag) return false;

  switch(m_tag) {
    case IMM_INT32:
    case IMM_MASK:
      return (m_intVal == rhs.m_intVal);
    case IMM_FLOAT32:
      return (m_floatVal == rhs.m_floatVal);
    default: assert(false);
  }

  return false;
}

}  // namespace V3DLib
