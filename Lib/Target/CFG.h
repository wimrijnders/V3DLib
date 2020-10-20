// Control-flow graphs (CFGs)

#ifndef _QPULIB_CFG_H_
#define _QPULIB_CFG_H_

#include "Common/Seq.h"
#include "Target/Syntax.h"

namespace QPULib {

// A set of successors.
typedef SmallSeq<InstrId> Succs;

// A CFG is a set of successors for each instruction.
typedef Seq<Succs> CFG;

// Function to construct a CFG.
void buildCFG(Seq<Instr> &instrs, CFG &cfg);

}  // namespace QPULib

#endif  // _QPULIB_CFG_H_
