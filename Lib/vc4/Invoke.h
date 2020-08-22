#ifndef _QPULIB_VC4_INVOKE_H_
#define _QPULIB_VC4_INVOKE_H_
#include <stdint.h>
#include "Common/Seq.h"
#include "Common/SharedArray.h"

namespace QPULib {

void invoke(
  int numQPUs,
  SharedArray<uint32_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params);

}  // namespace QPULib

#endif  // _QPULIB_VC4_INVOKE_H_
