#include "Invoke.h"
#include <stdio.h>
#include "Driver.h"


namespace QPULib {
namespace v3d {

void invoke(
  int numQPUs,
  SharedArray<uint64_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params) {

	assert(codeMem.size() != 0);

	SharedArray<uint32_t> unif(params->numElems + 3);
	SharedArray<uint32_t> done(1);
	done[0] = 0;

	// The first two slots in uniforms for vc4 are used for qpu number and num qpu's respectively
	//
	// We reuse these for the same functions in v3d, so as not to screw up the logic too much.
	// This does mean that two dummy values need to be passed in.
	int offset = 0;

	unif[offset++] = 0;  // qpu number (id for current qpu)
	unif[offset++] = 0;  // num qpu's running for this job

	for (int j = 0; j < params->numElems; j++) {
		unif[offset++] = params->elems[j];
	}

	// The last item is for the 'done' location;
	// Not sure if this is the correct slot to put it
	// TODO: scrutinize the python project for this
	unif[offset] = (uint32_t) done.getAddress();

/*
	Example from python project (search for more):

        f = pow(2, 25)

        code = drv.program(qpu_clock)
        unif = drv.alloc(2, dtype = 'uint32')
        done = drv.alloc(1, dtype = 'uint32')

        done[:] = 0

        unif[0] = f
        unif[1] = done.addresses()[0]
*/

	printf("============================\n");
	printf("This would run the v3d code!\n");
	printf("============================\n");

  Driver drv;

#if NOT_DONE_YET
	auto csd = drv.compute_shader_dispatcher();
	csd.dispatch(codeMem, &unif);
#endif // NOT_DONE_YET
}

}  // v3d
}  // QPULib
