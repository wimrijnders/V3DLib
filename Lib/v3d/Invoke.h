#ifndef _QPULIB_V3D_INVOKE_H
#define _QPULIB_V3D_INVOKE_H
#include "Common/SharedArray.h"
#include "Common/Seq.h"

namespace QPULib {
namespace v3d {

void invoke(
  int numQPUs,
  SharedArray<uint64_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params);

}  // v3d
}  // QPULib

#endif  // _QPULIB_V3D_INVOKE_H
