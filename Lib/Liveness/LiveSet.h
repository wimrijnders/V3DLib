#ifndef _V3DLIB_LIVENESS_LIVESET_H_
#define _V3DLIB_LIVENESS_LIVESET_H_
#include <string>
#include <set>
#include <vector>
#include "UseDef.h"

namespace V3DLib {

class Liveness;
class RegUsage;


/**
 * A live set contains the variables that are live-in to an instruction.
 */
class LiveSet : public std::set<RegId> {
  using Parent = std::set<RegId>;
public:
  void add_not_used(LiveSet const &def, UseDef const &use);
  void add(LiveSet const &rhs);
  void add(Set<RegId> const &rhs);
  std::string dump() const;
  bool member(RegId rhs) const;
};


class LiveSets {
public:
  UseDef useDefSet;

  LiveSets(int size);
  ~LiveSets();

  void init(Instr::List &instrs, Liveness &live);
  LiveSet &operator[](int index);
  std::vector<bool> possible_registers(int index, RegUsage &alloc, RegTag reg_tag = REG_A);

  static RegId choose_register(std::vector<bool> &possible, bool check_limit = true);  
  static void  dump_possible(std::vector<bool> &possible, int index = -1);

  std::string dump() const;

private:
  int m_size = 0;
  LiveSet *m_sets = nullptr;
};

}  // namespace V3DLib

#endif  //  _V3DLIB_LIVENESS_LIVESET_H_
