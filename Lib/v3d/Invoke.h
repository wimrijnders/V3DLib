#ifndef _VC6_INVOKE_H
#define _VC6_INVOKE_H
#include "SharedArray.h"
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

#endif // _VC6_INVOKE_H
