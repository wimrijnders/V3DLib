#ifndef _V3DLIB_TARGET_INSTR_REGORIMM_H_
#define _V3DLIB_TARGET_INSTR_REGORIMM_H_
#include "Reg.h"

namespace V3DLib {

class Imm;

struct EncodedSmallImm {
  int val;

  bool operator==(EncodedSmallImm const &rhs) const { return val == rhs.val;  }
  bool operator!=(EncodedSmallImm const &rhs) const { return !(*this == rhs); }
};


struct RegOrImm {
  RegOrImm() = default;
  RegOrImm(RegOrImm const &rhs) = default;
  RegOrImm(int rhs) { set_imm(rhs); }
  RegOrImm(Imm const &rhs);
  RegOrImm(Reg const &rhs);
  RegOrImm(Var const &rhs);

  RegOrImm &operator=(int rhs);
  RegOrImm &operator=(Imm const &rhs);
  RegOrImm &operator=(Reg const &rhs);

  bool operator==(RegOrImm const &rhs) const;
  bool operator!=(RegOrImm const &rhs) const { return !(*this == rhs); }
  bool operator==(Reg const &rhs) const;
  bool operator==(Imm const &rhs) const;

  bool is_reg() const { return m_is_reg;  }
  bool is_imm() const { return !m_is_reg; }
  std::string disp() const;

  Reg &reg();
  Reg reg() const;
  EncodedSmallImm &imm();
  EncodedSmallImm imm() const;

  bool is_transient() const;
  bool uses_src() const;

private:
  bool m_is_reg;        // if false, is an imm

  Reg m_reg;            // A register
  EncodedSmallImm m_smallImm;  // A small immediate

  void set_imm(int rhs);
  void set_reg(Reg const &rhs);
};


inline bool operator==(V3DLib::Reg const &lhs, V3DLib::RegOrImm const &rhs) { return rhs == lhs; } 
inline bool operator!=(V3DLib::Reg const &lhs, V3DLib::RegOrImm const &rhs) { return !(rhs == lhs); } 

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_INSTR_REGORIMM_H_
