#include "KernelDriver.h"
#include "vc4.h"

namespace QPULib {
namespace vc4 {

KernelDriver::KernelDriver() : QPULib::KernelDriver(Vc4Buffer) {}
KernelDriver::~KernelDriver() {}


void KernelDriver::kernelFinish() {
	QPULib::kernelFinish();  // QPU code to cleanly exit
}


/**
 * Encode target instrs into array of 32-bit ints
 */
void KernelDriver::encode(int numQPUs) {
	if (code.size() > 0) return;  // Don't bother if already encoded

	QPULib::encode(&m_targetCode, &code);
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t>* params) {
	debug("Called vc4 KernelDriver::invoke()");	
	assert(qpuCodeMem.size() == 0);
	assert(code.size() > 0);

	// Allocate memory for QPU code and parameters
	int numWords = code.numElems + 12*MAX_KERNEL_PARAMS + 12*2;

	qpuCodeMem.alloc(numWords);
	assert(qpuCodeMem.size() > 0);

	// Copy kernel to code memory
	int offset = 0;
	for (int i = 0; i < code.numElems; i++) {
		qpuCodeMem[offset++] = code.elems[i];
	}
	qpuCodeMemOffset = offset;

	enableQPUs();
	QPULib::invoke(numQPUs, qpuCodeMem, qpuCodeMemOffset, params);
	disableQPUs();
}

}  // namespace vc4
}  // namespace QPULib

