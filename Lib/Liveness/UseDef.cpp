#include "UseDef.h"

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class UseDefReg
///////////////////////////////////////////////////////////////////////////////

std::string UseDefReg::dump() const {
  std::string ret;

  ret << "(def: ";
  if (def.tag != NONE) {
    ret << def.dump() << ", ";
  }
  ret << "; ";

  ret << "use: ";
  for (auto const &reg : use) {
    ret << reg.dump() << ", ";
  }
  ret << ") ";

  return ret;
}


UseDefReg::UseDefReg(Instr const &instr, bool set_use_where) :
  use(instr.src_regs(set_use_where)),
  def(instr.dst_reg())
 {}

/*
bool UseDefReg::is_dest(Reg const &rhs) const {
  assert(rhs.tag != NONE);
  if (def.tag == NONE) return false;
  return (rhs == def);
}
*/

///////////////////////////////////////////////////////////////////////////////
// Class UseDef
///////////////////////////////////////////////////////////////////////////////

std::string UseDef::dump() const {
  std::string ret;

  ret << "(def: ";

  if (def.tag != NONE) {
   ret << def.dump();
  }

  ret << "; " 
      <<  "use: " << use.dump() << ") "; 

  return ret;
}


UseDef::UseDef(Instr const &instr, bool set_use_where) :
  use(instr.src_a_regs(set_use_where)),
  def(instr.dst_a_reg())
{}

}  // namespace V3DLib
