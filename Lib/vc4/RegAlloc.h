#ifndef _V3DLIB_REGALLOC_H_
#define _V3DLIB_REGALLOC_H_
#include "Target/Liveness.h"
#include "Target/instr/Instr.h"

namespace V3DLib {
namespace vc4 { 
void regAlloc(CFG* cfg, Seq<Instr>* instrs);

}  // namespace vc4; 
}  // namespace V3DLib

#endif  // _V3DLIB_REGALLOC_H_
