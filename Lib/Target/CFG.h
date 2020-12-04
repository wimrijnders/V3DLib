// Control-flow graphs (CFGs)

#ifndef _V3DLIB_CFG_H_
#define _V3DLIB_CFG_H_

#include "Common/Seq.h"
#include "Target/Syntax.h"

namespace V3DLib {

// A set of successors.
typedef SmallSeq<InstrId> Succs;

// A CFG is a set of successors for each instruction.
typedef Seq<Succs> CFG;

// Function to construct a CFG.
void buildCFG(Seq<Instr> &instrs, CFG &cfg);

}  // namespace V3DLib

#endif  // _V3DLIB_CFG_H_
