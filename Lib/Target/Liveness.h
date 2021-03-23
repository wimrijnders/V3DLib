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
 *  - Up to and including final use, or *till* (not including) next assignment.
 *    Note special case `x = f(x)`.
 *
 * This link follows the source code here pretty closely: https://lambda.uta.edu/cse5317/spring01/notes/node37.html
 *
 *  - "A variable x is *live* at a particular point (statement) in a program,
 *     if it holds a value that may be needed in the future.
 *
 *     That is, x is live at this point if there is a path (following gotos)
 *     from this point to a statement that use x and there is no assignment to x in any statement in the path."
 */
class Liveness {
public:
  Liveness(int numVars) : m_reg_usage(numVars) {}

  CFG const &cfg() const { return m_cfg; }
  int size() const { return m_set.size(); }
  RegUsage &reg_usage() { return m_reg_usage; }
  LiveSet &operator[](int index) { return get(index); }

  void compute(Instr::List &instrs);
  void computeLiveOut(InstrId i, LiveSet &liveOut);
  std::string dump();

  static void optimize(Instr::List &instrs, int numVars);

private:
  CFG          m_cfg;
  Seq<LiveSet> m_set;
  RegUsage     m_reg_usage;

  LiveSet &get(int index) { return m_set[index]; }
  void clear();
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


void allocate_registers(Instr::List &instrs, RegUsage const &alloc);

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_H_
