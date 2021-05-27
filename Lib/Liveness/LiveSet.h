#ifndef _V3DLIB_LIVENESS_LIVESET_H_
#define _V3DLIB_LIVENESS_LIVESET_H_
#include <string>
#include <vector>
//#include "UseDef.h"
#include "Target/instr/Instr.h"

namespace V3DLib {

class Liveness;
class RegUsage;


class LiveSets {
public:
  LiveSets(int size);
  ~LiveSets();

  void init(Instr::List &instrs, Liveness &live);
  RegIdSet &operator[](int index);
  std::vector<bool> possible_registers(int index, RegUsage &alloc, RegTag reg_tag = REG_A);

  static RegId choose_register(std::vector<bool> &possible, bool check_limit = true);  
  static void  dump_possible(std::vector<bool> &possible, int index = -1);

  std::string dump() const;

private:
  int m_size = 0;
  RegIdSet *m_sets = nullptr;
};

}  // namespace V3DLib

#endif  //  _V3DLIB_LIVENESS_LIVESET_H_
