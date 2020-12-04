#ifndef _V3DLIB_V3D_INVOKE_H
#define _V3DLIB_V3D_INVOKE_H
#include "Common/SharedArray.h"
#include "Common/Seq.h"

namespace V3DLib {
namespace v3d {

void invoke(
  int numQPUs,
  SharedArray<uint64_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t> &params);

}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INVOKE_H
