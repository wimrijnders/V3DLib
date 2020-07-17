#ifndef _VC6_INVOKE_H
#define _VC6_INVOKE_H
#include "Target/SharedArray.h"
#include "Common/SharedArray.h"
#include "Common/Seq.h"


namespace QPULib {
namespace vc6 {

//template <typename T>
//using SharedArray=QPULib::Target::SharedArray<T>;


void invoke(
  int numQPUs,
  Target::SharedArray<uint32_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params);

}  // vc6
}  // QPULib

#endif // _VC6_INVOKE_H
