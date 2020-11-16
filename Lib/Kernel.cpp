#include "Kernel.h"
#include "Target/Emulator.h"
#include "Target/CFG.h"
#include "Target/Liveness.h"
#include "Target/Pretty.h"

namespace QPULib {

std::vector<Ptr<Int>>   uniform_int_pointers;
std::vector<Ptr<Float>> uniform_float_pointers;


// ============================================================================
// Class KernelBase
// ============================================================================

int KernelBase::maxQPUs() {
	// TODO: better would be to take the values from Platform
	if (Platform::instance().has_vc4) {
		return 12;
	} else {
		return 8;
	}
}


void KernelBase::pretty(bool output_for_vc4, const char *filename) {
	if (output_for_vc4) {
		m_vc4_driver.pretty(numQPUs, filename);
	} else {
#ifdef QPU_MODE
		m_v3d_driver.pretty(numQPUs, filename);
#else
		fatal("KernelBase::pretty(): v3d code not generated for this platform.");
#endif
	}
}


/**
 * Invoke the emulator
 *
 * The emulator runs vc4 code.
 */
void KernelBase::emu() {
	assert(uniforms.size() != 0);
	emulate(numQPUs, &m_vc4_driver.targetCode(), numVars, uniforms, getBufferObject());
}


/**
 * Invoke the interpreter
 *
 * The interpreter parses the CFG ('source code') directly.
 */
void KernelBase::interpret() {
	assert(uniforms.size() != 0);
	interpreter(numQPUs, m_vc4_driver.sourceCode(), numVars, uniforms, getBufferObject());
}


#ifdef QPU_MODE
/**
 * Invoke kernel on physical QPU hardware
 */
void KernelBase::qpu() {
	assert(uniforms.size() != 0);

	if (Platform::instance().has_vc4) {
		m_vc4_driver.invoke(numQPUs, uniforms);
	} else {
		m_v3d_driver.invoke(numQPUs, uniforms);
	}
}
#endif  // QPU_MODE


/**
 * Invoke the kernel
 */
void KernelBase::call() {
#ifdef EMULATION_MODE
	emu();
#else
#ifdef QPU_MODE
	qpu();
#endif
#endif
};

}  // namespace QPULib
