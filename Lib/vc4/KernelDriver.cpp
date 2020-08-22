#include "KernelDriver.h"
#include "vc4.h"

namespace QPULib {
namespace vc4 {

KernelDriver::KernelDriver() : QPULib::KernelDriver(Vc4Buffer) {}
KernelDriver::~KernelDriver() {}


void KernelDriver::kernelFinish() {
	QPULib::kernelFinish();  // QPU code to cleanly exit
}


void KernelDriver::encode(int numQPUs, Seq<Instr> &targetCode) {
    // Encode target instrs into array of 32-bit ints
    Seq<uint32_t> code;
    QPULib::encode(&targetCode, &code);

    // Allocate memory for QPU code and parameters
    int numWords = code.numElems + 12*MAX_KERNEL_PARAMS + 12*2;

    qpuCodeMem.alloc(numWords);

    // Copy kernel to code memory
    int offset = 0;
    for (int i = 0; i < code.numElems; i++) {
      qpuCodeMem[offset++] = code.elems[i];
    }
    qpuCodeMemOffset = offset;
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t>* params) {
	assert(qpuCodeMem.size() > 0);
	enableQPUs();
	QPULib::invoke(numQPUs, qpuCodeMem, qpuCodeMemOffset, params);
	disableQPUs();
}

}  // namespace vc4
}  // namespace QPULib

