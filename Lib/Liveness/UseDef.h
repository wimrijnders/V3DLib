#ifndef _V3DLIB_LIVENESS_USEDEF_H_
#define _V3DLIB_LIVENESS_USEDEF_H_
#include <string>
#include "Common/Set.h"
#include "Target/instr/Instr.h"

namespace V3DLib {

struct UseDefReg {
  SmallSet<Reg> use;   // the variables used as src in an instruction
  SmallSet<Reg> def;   // the variables (at most 1!) used as dst by an instruction

  void set_used(Instr instr, bool set_use_where = false);
  bool is_dest(Reg const &rhs) const;
  bool is_src(Reg const &rhs) const;
  std::string dump() const;
};   

     
struct UseDef {
  SmallSet<RegId> use;
  SmallSet<RegId> def;

  void set_used(Instr const &instr, bool set_use_where = false);
  std::string dump() const;
};   

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_USEDEF_H_
