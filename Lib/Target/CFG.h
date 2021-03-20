//
// Control-flow graphs (CFGs)
//
// ============================================================================
#ifndef _V3DLIB_CFG_H_
#define _V3DLIB_CFG_H_
#include "Common/Set.h"
#include "Target/instr/Instr.h"

namespace V3DLib {

typedef int InstrId;                           // Index of instruction in instruction list
using Succs = SmallSet<InstrId>;               // Set of successors.

// Set of successors for each instruction.
class CFG : public Set<Succs> {
public:
  std::string dump() const {
    using ::operator<<;

    std::string ret;

    for (int i = 0; i < size(); ++i) {
      ret << i << ": " << (*this)[i] << "\n";
    } 

    return ret;
  }
};

void buildCFG(Instr::List &instrs, CFG &cfg);  // Function to construct a CFG.

}  // namespace V3DLib

#endif  // _V3DLIB_CFG_H_
