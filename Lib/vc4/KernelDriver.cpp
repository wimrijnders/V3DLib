#include "KernelDriver.h"

#ifdef QPU_MODE

namespace QPULib {
namespace vc4 {

KernelDriver::KernelDriver() : QPULib::KernelDriver(Vc4Buffer) {
}


KernelDriver::~KernelDriver() {
	delete qpuCodeMem;
}


void KernelDriver::kernelFinish() {
	// QPU code to cleanly exit
	QPULib::kernelFinish();
}


void KernelDriver::encode(int numQPUs, Seq<Instr> &targetCode) {
    // Encode target instrs into array of 32-bit ints
    Seq<uint32_t> code;
    QPULib::encode(&targetCode, &code);

    // Allocate memory for QPU code and parameters
    int numWords = code.numElems + 12*MAX_KERNEL_PARAMS + 12*2;

		qpuCodeMem = new vc4::SharedArray<uint32_t>;
    qpuCodeMem->alloc(numWords);

    // Copy kernel to code memory
    int offset = 0;
    for (int i = 0; i < code.numElems; i++) {
      (*qpuCodeMem)[offset++] = code.elems[i];
    }
    qpuCodeMemOffset = offset;
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t>* params) {
	assert(qpuCodeMem != nullptr);
	enableQPUs();
	QPULib::invoke(numQPUs, *qpuCodeMem, qpuCodeMemOffset, params);
	disableQPUs();
}

}  // namespace vc4
}  // namespace QPULib

#endif  // QPU_MODE
