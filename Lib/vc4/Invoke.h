#ifndef _V3DLIB_VC4_INVOKE_H_
#define _V3DLIB_VC4_INVOKE_H_
#include <stdint.h>
#include "Common/Seq.h"
#include "Common/SharedArray.h"

namespace V3DLib {

void init_uniforms(Data &uniforms, IntList const &params, int numQPUs);
void init_launch_messages(Data &launch_messages, Code const &code, IntList const &params, Data const &uniforms);
void invoke(int numQPUs, Data const &launch_messages); 

}  // namespace V3DLib

#endif  // _V3DLIB_VC4_INVOKE_H_
