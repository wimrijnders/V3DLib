//
// Liveness analysis
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_LIVENESS_H_
#define _V3DLIB_LIVENESS_H_
#include <string>
#include <vector>
#include "CFG.h"
#include "liveness/RegUsage.h"
#include "liveness/UseDef.h"

namespace V3DLib {

/**
 * A live set contains the variables that are live-in to an instruction.
 */
class LiveSet : public SmallSet<RegId> {
 using Parent = SmallSet<RegId>;

public:
  void add_not_used(LiveSet const &def, UseDef const &use);
  std::string dump() const;
};


/**
 * The result of liveness analysis is a set of live variables for each instruction.
 *
 * `Liveness Analysis` is a method to assign registers to variables.
 * In the period that a variable is `live`, one register is exclusively used for
 * that variable. When not live, the register can be reassigned to another live variable.
 *
 * A variable is 'live' in the instruction list:
 *  - from 1 *after* an assignment
 *  - till final use
 *
 * This link follows the source code here pretty closely: https://lambda.uta.edu/cse5317/spring01/notes/node37.html
 */
class Liveness {
public:
  Liveness(CFG &cfg, int numVars) : m_cfg(cfg), reg_usage(numVars) {}

  int size() const { return m_set.size(); }
  RegUsage &alloc() { return reg_usage; }
  LiveSet &operator[](int index) { return get(index); }

  void compute(Instr::List &instrs);
  void computeLiveOut(InstrId i, LiveSet &liveOut);
  std::string dump();

private:
  CFG &m_cfg;
  Seq<LiveSet> m_set;
  RegUsage reg_usage;

  LiveSet &get(int index) { return m_set[index]; }
  void compute_liveness(Instr::List &instrs);
  void setSize(int size);
  bool insert(int index, LiveSet const &set);
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

private:
  int m_size = 0;
  LiveSet *m_sets = nullptr;
};


void introduceAccum(CFG &cfg, Instr::List &instrs, int numVars);
void allocate_registers(Instr::List &instrs, RegUsage const &alloc);

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_H_
