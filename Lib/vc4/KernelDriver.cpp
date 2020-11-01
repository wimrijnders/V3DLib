#include "KernelDriver.h"
#include "vc4.h"
#include "Encode.h"
#include "DMA.h"
#include "dump_instr.h"

namespace QPULib {
namespace vc4 {

KernelDriver::KernelDriver() : QPULib::KernelDriver(Vc4Buffer) {}


/**
 * Add the postfixi code to the kernel.
 *
 * Note that this emits kernel code.
 */
void KernelDriver::kernelFinish() {
  // Ensure outstanding DMAs have completed
  dmaWaitRead();
  dmaWaitWrite();

  // QPU 0 waits until all other QPUs have finished
  // before sending a host IRQ.
  If (me() == 0)
    Int n = numQPUs()-1;
    For (Int i = 0, i < n, i++)
      semaDec(15);
    End
    hostIRQ();
  Else
    semaInc(15);
  End
}


/**
 * Encode target instrs into array of 32-bit ints
 */
void KernelDriver::encode(int numQPUs) {
	if (code.size() > 0) return;  // Don't bother if already encoded

	QPULib::vc4::encode(&m_targetCode, &code);
}


void KernelDriver::emit_opcodes(FILE *f) {
	fprintf(f, "Opcodes for vc4\n");
	fprintf(f, "===============\n\n");
	fflush(f);

	Seq<uint64_t> instructions;

	for (int i = 0; i < m_targetCode.size(); ++i ) {
		instructions << vc4::encode(m_targetCode[i]);
	}

	dump_instr(f, instructions.data(), instructions.size());
}


void KernelDriver::invoke_intern(int numQPUs, Seq<int32_t>* params) {
	//debug("Called vc4 KernelDriver::invoke()");	
	assert(code.size() > 0);

	int numWords = code.size() + 12*MAX_KERNEL_PARAMS + 12*2;

	// Assumption: code in a kernel, once allocated, doesnt' change
	if (qpuCodeMem.allocated()) {
		//debug("vc4 KernelDriver::invoke() code and parameters memory already allocated");
		assert(qpuCodeMem.size() == numWords);
	} else {
		// Allocate memory for QPU code and parameters

		qpuCodeMem.alloc(numWords);
		assert(qpuCodeMem.size() > 0);

		// Copy kernel to code memory
		int offset = 0;
		for (int i = 0; i < code.size(); i++) {
			qpuCodeMem[offset++] = code[i];
		}
		qpuCodeMemOffset = offset;
	}

	enableQPUs();
	QPULib::invoke(numQPUs, qpuCodeMem, qpuCodeMemOffset, params);
	disableQPUs();
}

}  // namespace vc4
}  // namespace QPULib

