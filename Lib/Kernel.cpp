#include "Kernel.h"
#include "Source/Translate.h"
#include "Target/Emulator.h"
#include "Target/RemoveLabels.h"
#include "Target/CFG.h"
#include "Target/Liveness.h"
#include "Target/ReachingDefs.h"
#include "Target/LiveRangeSplit.h"
#include "Target/RegAlloc.h"
#include "Target/Satisfy.h"
#include "Target/LoadStore.h"
#include "Target/Encode.h"
#include "Target/Pretty.h"

namespace QPULib {


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
		m_vc4_driver.encode(numQPUs);
		m_vc4_driver.pretty(filename);
	} else {
#ifdef QPU_MODE
		m_v3d_driver.encode(numQPUs);
		m_v3d_driver.pretty(filename);
#else
		fatal("KernelBase::pretty(): v3d code not generated for this platform.");
#endif
	}
}


void KernelBase::init_compile() {
	controlStack.clear();
	stmtStack.clear();         // Needs to be run before getUniformInt() below
	stmtStack.push(mkSkip());  // idem
	resetFreshVarGen();
	resetFreshLabelGen();

	// Reserved general-purpose variables
	Int qpuId, qpuCount;
	qpuId = getUniformInt();
	qpuCount = getUniformInt();
}


void KernelBase::invoke_qpu(QPULib::KernelDriver &kernel_driver) {
	kernel_driver.encode(numQPUs);
	if (!kernel_driver.handle_errors()) {
   	// Invoke kernel on QPUs
		kernel_driver.invoke(numQPUs, &uniforms);
	}
}


void KernelBase::emu() {
	assert(uniforms.size() != 0);

	// Emulator runs the vc4 code
	emulate(numQPUs, &m_vc4_driver.targetCode(), numVars, &uniforms, getBufferObject());
}


/**
 * Invoke the interpreter
 */
void KernelBase::interpret() {
	assert(uniforms.size() != 0);

	// Interpreter runs the vc4 code
	interpreter(numQPUs, m_vc4_driver.sourceCode(), numVars, &uniforms, getBufferObject());
}


#ifdef QPU_MODE
/**
 * Invoke kernel on physical QPU hardware
 */
void KernelBase::qpu() {
	assert(uniforms.size() != 0);

	if (Platform::instance().has_vc4) {
		invoke_qpu(m_vc4_driver);
	} else {
		invoke_qpu(m_v3d_driver);
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


// ============================================================================
// Compile kernel
// ============================================================================

/**
 * @param body        top of the AST
 * @param targetCode  output variable for the target code assembled from the AST and adjusted
 */
void compileKernel(Seq<Instr>* targetCode, Stmt* body) {
	// At this point, the target code contains just the uniform loads
	// This is a good location to add further initialization code
	*targetCode << getSourceTranslate().translate_add_init();

  // Translate to target code
  translateStmt(targetCode, body);

  // Load/store pass
  loadStorePass(targetCode);

  // Construct control-flow graph
  CFG cfg;
  buildCFG(targetCode, &cfg);

  // Apply live-range splitter
  //liveRangeSplit(targetCode, &cfg);

  // Perform register allocation
  regAlloc(&cfg, targetCode);

  // Satisfy target code constraints
//breakpoint
  satisfy(targetCode);

  // Translate branch-to-labels to relative branches
  removeLabels(targetCode);
}

}  // namespace QPULib
