#ifndef _V3DLIB_VC4_REGALLOC_H_
#define _V3DLIB_VC4_REGALLOC_H_
#include "Target/instr/Instr.h"
#include "Liveness/Liveness.h"

namespace V3DLib {
namespace vc4 {

void regAlloc(Instr::List &instrs);

}  // namespace vc4; 
}  // namespace V3DLib

#endif  // _V3DLIB_VC4_REGALLOC_H_
