#ifndef _V3DLIB_LIVENESS_USEDEF_H_
#define _V3DLIB_LIVENESS_USEDEF_H_
#include <string>
#include <set>
#include "Common/Set.h"
#include "Target/instr/Instr.h"

namespace V3DLib {

struct UseDefReg {
  std::set<Reg> use;   // the variables used as src in an instruction
  std::set<Reg> def;   // the variables (at most 1!) used as dst by an instruction

  UseDefReg(Instr const &instr, bool set_use_where = false);

  bool is_dest(Reg const &rhs) const { return (def.find(rhs) != def.end()); }
  bool is_src(Reg const &rhs)  const { return (use.find(rhs) != def.end()); }
  std::string dump() const;
};   


class RegIdSet : public std::set<RegId> {
public:
  void add(RegIdSet const &rhs);
  void remove(RegIdSet const &rhs);
  bool member(RegId rhs) const { return (find(rhs) != cend()); }
  RegId first() const;
  std::string dump() const;
};

     
struct UseDef {
  RegIdSet use;
  RegIdSet def;

  UseDef(Instr const &instr, bool set_use_where = false);
  std::string dump() const;

private:
  void set_used(Instr const &instr, bool set_use_where = false);
};   

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_USEDEF_H_
