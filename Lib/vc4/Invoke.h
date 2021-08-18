#ifndef _V3DLIB_VC4_INVOKE_H_
#define _V3DLIB_VC4_INVOKE_H_
#include <stdint.h>
#include "Common/Seq.h"
#include "Common/SharedArray.h"

namespace V3DLib {

using Code   = SharedArray<uint64_t>;
using Data   = SharedArray<uint32_t>;

void invoke(int numQPUs, Code &codeMem, IntList const &params, Data &uniforms, Data &launch_messages); 

}  // namespace V3DLib

#endif  // _V3DLIB_VC4_INVOKE_H_
