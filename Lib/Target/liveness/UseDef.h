#ifndef _V3DLIB_LIVENESS_USEDEF_H_
#define _V3DLIB_LIVENESS_USEDEF_H_
#include <string>
#include "Common/Set.h"
#include "../instr/Instr.h"

namespace V3DLib {

// 'use' and 'def' sets:
//   * 'use' set: the variables read by an instruction
//   * 'def' set: the variables modified by an instruction

struct UseDefReg {
  SmallSet<Reg> use;
  SmallSet<Reg> def;

  std::string dump() const;
};   

     
struct UseDef {
  SmallSet<RegId> use;
  SmallSet<RegId> def;

  void set_used(Instr const &instr, bool set_use_where = false);
  std::string dump() const;
};   

void useDefReg(Instr instr, UseDefReg* out, bool set_use_where = false);

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_USEDEF_H_
