#include "KernelDrivers.h"
#include "Target/Encode.h"
#include "VideoCore/VideoCore.h"
#include "VideoCore/Invoke.h"
#include "Source/Stmt.h"
#include "v3d/Invoke.h"


namespace QPULib {

namespace vc4 {
using namespace VideoCore;

KernelDriver::KernelDriver() {
	enableQPUs();

	qpuCodeMem = new SharedArray<uint32_t>;
}


KernelDriver::~KernelDriver() {
	disableQPUs();
	delete qpuCodeMem;
}


void KernelDriver::kernelFinish() {
	// QPU code to cleanly exit
	QPULib::kernelFinish();
}


void KernelDriver::encode(Seq<Instr> &targetCode) {
    // Encode target instrs into array of 32-bit ints
    Seq<uint32_t> code;
    QPULib::encode(&targetCode, &code);

    // Allocate memory for QPU code and parameters
    int numWords = code.numElems + 12*MAX_KERNEL_PARAMS + 12*2;
    qpuCodeMem->alloc(numWords);

    // Copy kernel to code memory
    int offset = 0;
    for (int i = 0; i < code.numElems; i++) {
      (*qpuCodeMem)[offset++] = code.elems[i];
    }
    qpuCodeMemOffset = offset;
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t>* params) {
	QPULib::invoke(numQPUs, *qpuCodeMem, qpuCodeMemOffset, params);
}

}  // namespace vc4


namespace v3d {


KernelDriver::KernelDriver() {
	qpuCodeMem = new SharedArray<uint64_t>;
}

KernelDriver::~KernelDriver() {
	delete qpuCodeMem;
}


void KernelDriver::encode(Seq<Instr> &targetCode) {
	assert(false);  // TODO
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t>* params) {
	v3d::invoke(numQPUs, *qpuCodeMem, qpuCodeMemOffset, params);
}

}  // namespace v3d
}  // namespace QPULib


