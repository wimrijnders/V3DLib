#include "Invoke.h"
#include <stdio.h>
#include "Driver.h"


namespace QPULib {
namespace v3d {

void invoke(
  int numQPUs,
  SharedArray<uint64_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t> &params) {

	assert(codeMem.size() != 0);

	SharedArray<uint32_t> unif(params.size() + 3);
	SharedArray<uint32_t> done(1);
	done[0] = 0;

	// The first two slots in uniforms for vc4 are used for qpu number and num qpu's respectively
	// We do the same for v3d, so as not to screw up the logic too much.
	int offset = 0;

	unif[offset++] = 0;        // qpu number (id for current qpu) - 0 is for 1 QPU
	unif[offset++] = numQPUs;  // num qpu's running for this job

	for (int j = 0; j < params.size(); j++) {
		unif[offset++] = params[j];
	}

	// The last item is for the 'done' location;
	// Not sure if this is the correct slot to put it
	// TODO: scrutinize the python project for this
	unif[offset] = (uint32_t) done.getAddress();

  Driver drv;
	drv.add_bo(getBufferObject());
	drv.execute(codeMem, &unif, numQPUs);
}

}  // v3d
}  // QPULib
