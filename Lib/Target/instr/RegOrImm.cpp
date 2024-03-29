#include "RegOrImm.h"
#include "Support/basics.h"
#include "Support/Platform.h"
#include "Target/SmallLiteral.h"
#include "Imm.h"
#include "v3d/instr/SmallImm.h"

namespace V3DLib {

RegOrImm::RegOrImm(Imm const &rhs) { set_imm(rhs.encode_imm()); }
RegOrImm::RegOrImm(Var const &rhs) { set_reg(rhs); }
RegOrImm::RegOrImm(Reg const &rhs) { set_reg(rhs); }

Reg &RegOrImm::reg()                  { assert(is_reg()); return m_reg; }
Reg RegOrImm::reg() const             { assert(is_reg()); return m_reg; }
EncodedSmallImm &RegOrImm::imm()      { assert(is_imm()); return m_smallImm; }
EncodedSmallImm RegOrImm::imm() const { assert(is_imm()); return m_smallImm; }

void RegOrImm::set_imm(int rhs) {
  // input should be in the encode range for target platforms
  assert((Platform::compiling_for_vc4()  && 0 <= rhs && rhs <= 47)
      || (!Platform::compiling_for_vc4() && v3d::instr::SmallImm::is_legal_encoded_value(rhs))
  );

  m_is_reg  = false;
  m_smallImm.val = rhs;
}


void RegOrImm::set_reg(Reg const &rhs) {
  m_is_reg  = true;
  m_reg = rhs;
  m_reg.can_read(true);
}


RegOrImm &RegOrImm::operator=(int rhs)        { set_imm(rhs); return *this; }
RegOrImm &RegOrImm::operator=(Imm const &rhs) { set_imm(rhs.encode_imm()); return *this; }
RegOrImm &RegOrImm::operator=(Reg const &rhs) { set_reg(rhs); return *this; }


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

  int rhs_encoded = rhs.encode_imm();
  if (rhs_encoded == Imm::INVALID_ENCODING) return false; 

  return m_smallImm.val == rhs_encoded;
}


bool RegOrImm::can_read(bool check) const {
  if (m_is_reg) return m_reg.can_read(check);

  return true;
}


std::string RegOrImm::disp() const {
  if (m_is_reg) {
    return m_reg.dump();
  } else {
    if (Platform::compiling_for_vc4()) {
      return printSmallLit(m_smallImm.val);
    } else {
      return v3d::instr::SmallImm::print_encoded_value(m_smallImm.val);
    }
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
