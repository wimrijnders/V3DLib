#include "Kernel.h"
#include "Support/basics.h"
#include "Target/Emulator.h"
#include "Target/CFG.h"
#include "Target/Liveness.h"
#include "Target/Pretty.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

// ============================================================================
// Class KernelBase
// ============================================================================

void KernelBase::pretty(bool output_for_vc4, const char *filename, bool output_qpu_code) {
  if (output_for_vc4) {
    m_vc4_driver.pretty(numQPUs, filename, output_qpu_code);
  } else {
#ifdef QPU_MODE
    m_v3d_driver.pretty(numQPUs, filename, output_qpu_code);
#else
    warning("KernelBase::pretty(): v3d code not generated for this platform.");
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
  emulate(numQPUs, m_vc4_driver.targetCode(), numVars, uniforms, getBufferObject());
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

  if (Platform::has_vc4()) {
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
  if (Platform::use_main_memory()) {
    warning("Main memory selected in QPU mode, running on emulator instead of QPU.");
    emu();
  } else {
    qpu();
  }
#endif
#endif
};


std::string KernelBase::compile_info() const {
  std::string ret;

  ret << "\n"
      << "Compile info\n"
      << "============\n"
      << "vc4 compile generated " << numVars << " variables.\n";

#ifdef QPU_MODE
  ret << "v3d compile generated " << getFreshVarCount() << " variables.\n";
#endif  // QPU_MODE

  return ret;
}

}  // namespace V3DLib
