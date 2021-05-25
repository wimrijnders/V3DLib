//
// Liveness analysis
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_LIVENESS_LIVENESS_H_
#define _V3DLIB_LIVENESS_LIVENESS_H_
#include <string>
#include <vector>
#include "CFG.h"
#include "RegUsage.h"
#include "LiveSet.h"

namespace V3DLib {

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
  RegIdSet &operator[](int index) { return get(index); }

  void compute(Instr::List &instrs);
  void computeLiveOut(InstrId i, RegIdSet &liveOut);
  std::string dump();

  static void optimize(Instr::List &instrs, int numVars);

private:
  CFG          m_cfg;
  Seq<RegIdSet> m_set;
  RegUsage     m_reg_usage;

  RegIdSet &get(int index) { return m_set[index]; }
  void clear();
  void compute_liveness(Instr::List &instrs);
  void setSize(int size);
  bool insert(int index, RegIdSet const &set);
};


void allocate_registers(Instr::List &instrs, RegUsage const &alloc);

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_LIVENESS_H_
