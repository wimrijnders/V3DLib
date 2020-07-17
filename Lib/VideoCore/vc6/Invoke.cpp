#include "Invoke.h"
#include <stdio.h>
#include "Driver.h"


namespace QPULib {
namespace vc6 {
  using SharedArray = Target::SharedArray<uint32_t>;

void invoke(
  int numQPUs,
  SharedArray &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params) {

	printf("============================\n");
	printf("This would run the vc6 code!\n");
	printf("============================\n");

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
}

}  // vc6
}  // QPULib
