#include "UseDef.h"

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class UseDefReg
///////////////////////////////////////////////////////////////////////////////

std::string UseDefReg::dump() const {
  std::string ret;

  ret << "(def: ";
  for (auto const &reg : def) {
    ret << reg.dump() << ", ";
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
  def(instr.dst_regs())
 {}


///////////////////////////////////////////////////////////////////////////////
// Class RegIdSet
///////////////////////////////////////////////////////////////////////////////

void RegIdSet::add(RegIdSet const &rhs) {
  insert(rhs.begin(), rhs.end());
}


void RegIdSet::remove(RegIdSet const &use ) {
  // NOT WORKING
  // Screws up use and thereby screwing up *this as well
  //erase(use.begin(), use.end());

  for (auto r : use) {
    erase(r);
  }
}


RegId RegIdSet::first() const { assert(!empty()); return *cbegin(); }


std::string RegIdSet::dump() const {
  std::string ret;

  ret << "(";

  for (auto reg : *this) {
    ret << reg << ", ";
  }

  ret << ")";

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Class UseDef
///////////////////////////////////////////////////////////////////////////////

std::string UseDef::dump() const {
  std::string ret;

  ret << "(def: " << def.dump() << "; " 
      <<  "use: " << use.dump() << ") "; 

  return ret;
}


UseDef::UseDef(Instr const &instr, bool set_use_where) {
  for (auto const &r : instr.src_regs(set_use_where)) {
    if (r.tag == REG_A) use.insert(r.regId);
  }

  for (auto const &r : instr.dst_regs()) {
    if (r.tag == REG_A) def.insert(r.regId);
  }
}

/**
 * Get variables used in instruction
 *
 * Same as `useDefReg()`, except only yields ids of registers in register file A.
 */
void UseDef::set_used(Instr const &instr, bool set_use_where) {
  use.clear();
  def.clear();

  for (auto const &r : instr.src_regs(set_use_where)) {
    if (r.tag == REG_A) use.insert(r.regId);
  }

  for (auto const &r : instr.dst_regs()) {
    if (r.tag == REG_A) def.insert(r.regId);
  }
}

}  // namespace V3DLib
