#ifndef _V3DLIB_VC4_INVOKE_H_
#define _V3DLIB_VC4_INVOKE_H_
#include <stdint.h>
#include "Common/Seq.h"
#include "Common/SharedArray.h"

namespace V3DLib {

void invoke(int numQPUs, SharedArray<uint32_t> &codeMem, int qpuCodeMemOffset, IntList *params); 

}  // namespace V3DLib

#endif  // _V3DLIB_VC4_INVOKE_H_
