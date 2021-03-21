#include "Imm.h"
#include "Support/basics.h"

namespace V3DLib {

Imm::Imm(int i)   : m_tag(IMM_INT32),   m_intVal(i) {}
Imm::Imm(float f) : m_tag(IMM_FLOAT32), m_floatVal(f) {}

int   Imm::intVal()   const { assert(m_tag == IMM_INT32);   return m_intVal; }
int   Imm::mask()     const { assert(m_tag == IMM_MASK);    return m_intVal; }
float Imm::floatVal() const { assert(m_tag == IMM_FLOAT32); return m_floatVal; }

bool Imm::is_zero() const { return m_tag == IMM_INT32 && m_intVal == 0; }

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


}  // namespace V3DLib
