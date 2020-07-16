#include "Invoke.h"
#include <stdio.h>


namespace QPULib {
namespace vc6 {

void invoke(
  int numQPUs,
  SharedArray<uint32_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params) {

	printf("============================\n");
	printf("This would run the vc6 code!\n");
	printf("============================\n");
}

}  // vc6
}  // QPULib
