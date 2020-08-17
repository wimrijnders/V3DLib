#ifdef QPU_MODE

#ifndef _QPULIB_VC4_INVOKE_H_
#define _QPULIB_VC4_INVOKE_H_

#include "Common/Seq.h"
#include "SharedArray.h"
#include <stdint.h>

namespace QPULib {

void invoke(
  int numQPUs,
  vc4::SharedArray<uint32_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params);

}  // namespace QPULib

#endif  // _QPULIB_VC4_INVOKE_H_
#endif  // QPU_MODE
