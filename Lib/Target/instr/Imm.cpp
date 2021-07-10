#include "Imm.h"
#include "Support/basics.h"
#include "Support/Platform.h"
#include "v3d/instr/SmallImm.h"
#include "Target/SmallLiteral.h"

namespace V3DLib {

Imm::Imm(int i)      : m_tag(IMM_INT32),   m_intVal(i) {}
Imm::Imm(unsigned i) : m_tag(IMM_INT32),   m_intVal((int) i) {}
Imm::Imm(float f)    : m_tag(IMM_FLOAT32), m_floatVal(f) {}

int   Imm::intVal()   const { assert(m_tag == IMM_INT32);   return m_intVal; }
int   Imm::mask()     const { assert(m_tag == IMM_MASK);    return m_intVal; }
float Imm::floatVal() const { assert(m_tag == IMM_FLOAT32); return m_floatVal; }

bool Imm::is_zero() const { return m_tag == IMM_INT32 && m_intVal == 0; }


/**
 * Check if this is an immediate value which does not
 * need to be constructed inline.
 */
bool Imm::is_basic() const {
  return INVALID_ENCODING != encode_imm();
}


/**
 * Return encoded small value for immediate, if possible
 *
 * @return Encoded value if encoding possible, -17 otherwise
 */
int Imm::encode_imm() const {
  assert(m_tag != IMM_MASK);  // Not dealing with this here
  int dummy = INVALID_ENCODING;  // Invalid value for both v3d and vc4

  if (Platform::compiling_for_vc4()) {
    if (is_int()) {
      dummy = encodeSmallInt(m_intVal);
    } else if (is_float()) {
      dummy = encodeSmallFloat(m_floatVal);
    }

    if (dummy == -1) dummy = INVALID_ENCODING;
  } else {
    if (is_int()) {
      if (!v3d::instr::SmallImm::int_to_opcode_value(m_intVal, dummy)) {
        dummy = INVALID_ENCODING;
      }
    } else if (is_float()) {
      if (!v3d::instr::SmallImm::float_to_opcode_value(m_floatVal, dummy)) {
        dummy = INVALID_ENCODING;
      }
    }
  }

  return dummy;
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


/**
 * Return a representation of the value which is suitable for adding
 * to a (vc4) instruction
 */
uint32_t Imm::encode() const {
  assert(sizeof(m_intVal)   == sizeof(uint32_t));
  assert(sizeof(m_floatVal) == sizeof(uint32_t));

  if (m_tag == IMM_FLOAT32) {
    auto *tmp = (uint32_t *) &m_floatVal;
    return *tmp;
  }

  return (uint32_t) m_intVal;
}

}  // namespace V3DLib
