#include "Invoke.h"
#include <stdio.h>
#include "Driver.h"


namespace QPULib {
namespace v3d {

void invoke(
  int numQPUs,
  SharedArray<uint32_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params) {

	printf("============================\n");
	printf("This would run the v3d code!\n");
	printf("============================\n");

#if NOT_DONE_YET
	assert(codeMem.size() != 0);

  Driver drv;

	SharedArray unif(params->numElems + 1);
	SharedArray done(1);
	done[0] = 0;

	for (int j = 0; j < params->numElems; j++) {
		unif[j] = params->elems[j];
	}
	unif[params->numElems] = (uint32_t) done.getAddress();

/*
        f = pow(2, 25)

        code = drv.program(qpu_clock)
        unif = drv.alloc(2, dtype = 'uint32')
        done = drv.alloc(1, dtype = 'uint32')

        done[:] = 0

        unif[0] = f
        unif[1] = done.addresses()[0]
*/
	auto csd = drv.compute_shader_dispatcher();
	csd.dispatch(codeMem, &unif);
#endif // NOT_DONE_YET
}

}  // v3d
}  // QPULib
