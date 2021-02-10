//
// Control-flow graphs (CFGs)
//
// ============================================================================
#ifndef _V3DLIB_CFG_H_
#define _V3DLIB_CFG_H_
#include "Target/instr/Instr.h"

namespace V3DLib {

typedef int InstrId;  // Index of instruction in instruction list

// A set of successors.
using Succs =  SmallSeq<InstrId>;

// A CFG is a set of successors for each instruction.
using CFG =  Seq<Succs>;

// Function to construct a CFG.
void buildCFG(Seq<Instr> &instrs, CFG &cfg);

}  // namespace V3DLib

#endif  // _V3DLIB_CFG_H_
