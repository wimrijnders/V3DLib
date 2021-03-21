#include "UseDef.h"

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class UseDefReg
///////////////////////////////////////////////////////////////////////////////

std::string UseDefReg::dump() const {
  std::string ret;

  ret << "(def: ";
  for (int j = 0; j < def.size(); j++) {
    ret << def[j].dump();
  }
  ret << "; ";

  ret << "use: ";
  for (int j = 0; j < use.size(); j++) {
    ret << use[j].dump();
  }
  ret << ") ";

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Class UseDef
///////////////////////////////////////////////////////////////////////////////

std::string UseDef::dump() const {
  std::string ret;

  ret << "(def: ";
  for (int j = 0; j < def.size(); j++) {
    ret << def[j];
  }
  ret << "; ";

  ret << "use: ";
  for (int j = 0; j < use.size(); j++) {
    ret << use[j];
  }
  ret << ") ";

  return ret;
}


/**
 * Get variables used in instruction
 *
 * Same as `useDefReg()`, except only yields ids of registers in register file A.
 */
void UseDef::set_used(Instr const &instr, bool set_use_where) {
  UseDefReg set;
  useDefReg(instr, &set, set_use_where);
  use.clear();
  def.clear();

  for (int i = 0; i < set.use.size(); i++) {
    Reg r = set.use[i];
    if (r.tag == REG_A) use.insert(r.regId);
  }

  for (int i = 0; i < set.def.size(); i++) {
    Reg r = set.def[i];
    if (r.tag == REG_A) def.insert(r.regId);
  }
}

}  // namespace V3DLib
