#ifndef _V3DLIB_LIVENESS_USEDEF_H_
#define _V3DLIB_LIVENESS_USEDEF_H_
#include <string>
#include <set>
#include "Target/instr/Instr.h"

namespace V3DLib {

struct UseDefReg {
  std::set<Reg> use;   // the variables used as src in an instruction
  Reg  def;            // the variable (at most 1, absence indicated by NONE tag)) used as dst by an instruction

  UseDefReg(Instr const &instr, bool set_use_where = false);

  std::string dump() const;
};   



     
/**
 * Get variables used in instruction
 *
 * Same as `useDefReg`, except only yields ids of registers in register file A.
 */
struct UseDef {
  RegIdSet use;
  Reg def;

  UseDef(Instr const &instr, bool set_use_where = false);
  std::string dump() const;
};   

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_USEDEF_H_
