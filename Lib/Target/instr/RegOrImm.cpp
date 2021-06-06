#include "RegOrImm.h"
#include "Support/basics.h"
#include "Target/SmallLiteral.h"
#include "Imm.h"

namespace V3DLib {

Reg &RegOrImm::reg()           { assert(is_reg()); return m_reg; }
Reg RegOrImm::reg() const      { assert(is_reg()); return m_reg; }
SmallImm &RegOrImm::imm()      { assert(is_imm()); return m_smallImm; }
SmallImm RegOrImm::imm() const { assert(is_imm()); return m_smallImm; }

void RegOrImm::set_imm(int rhs) {
  m_is_reg  = false;
  m_smallImm.val = rhs;
}


void RegOrImm::set_reg(RegTag tag, RegId id) {
  m_is_reg  = true;
  m_reg.tag   = tag;
  m_reg.regId = id;
}


void RegOrImm::set_reg(Reg const &rhs) {
  m_is_reg  = true;
  m_reg = rhs;
}


bool RegOrImm::operator==(RegOrImm const &rhs) const {
  if (m_is_reg != rhs.m_is_reg) return false;

  if (m_is_reg) {
    return m_reg == rhs.m_reg;
  } else {
    return m_smallImm == rhs.m_smallImm;
  }
}


bool RegOrImm::operator==(Reg const &rhs) const {
  if (!m_is_reg) return false;
  return m_reg == rhs;
}


bool RegOrImm::operator==(Imm const &rhs) const {
  if (m_is_reg) return false;
  if (rhs.is_float()) return false;
  return m_smallImm.val == rhs.intVal();
}


std::string RegOrImm::disp() const {
  if (m_is_reg) {
    return m_reg.dump();
  } else {
    return printSmallLit(m_smallImm.val);
  }
}


bool RegOrImm::is_transient() const {
  if (is_imm()) return false;
  return (reg().tag == NONE || reg().tag == TMP_A || reg().tag == TMP_B);
}


bool RegOrImm::uses_src() const {
  if (is_imm()) return true;
  return (reg().tag == REG_A || reg().tag == REG_B);
}

}  // namespace V3DLib
