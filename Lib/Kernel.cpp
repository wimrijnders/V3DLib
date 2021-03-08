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

KernelDriver &KernelBase::select_driver(bool output_for_vc4) {
  if (output_for_vc4) {
    return m_vc4_driver;
  } else {
#ifdef QPU_MODE
    return m_v3d_driver;
#else
    warning("KernelBase::pretty(): v3d code not generated for this platform.");
#endif
  }
}


void KernelBase::pretty(bool output_for_vc4, const char *filename, bool output_qpu_code) {
	select_driver(output_for_vc4).pretty(numQPUs, filename, output_qpu_code);
}


/**
 * Invoke the emulator
 *
 * The emulator runs vc4 code.
 */
void KernelBase::emu() {
  assert(uniforms.size() != 0);
  emulate(numQPUs, m_vc4_driver.targetCode(), m_vc4_driver.numVars(), uniforms, getBufferObject());
}


/**
 * Invoke the interpreter
 *
 * The interpreter parses the CFG ('source code') directly.
 */
void KernelBase::interpret() {
  assert(uniforms.size() != 0);
  interpreter(numQPUs, m_vc4_driver.sourceCode(), m_vc4_driver.numVars(), uniforms, getBufferObject());
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
      << "vc4 compile num generated variables: " << m_vc4_driver.numVars() << "\n"
      << "vc4 num accs introduced            : " << m_vc4_driver.numAccs() << "\n";

#ifdef QPU_MODE
  ret << "v3d compile num generated variables: " << m_v3d_driver.numVars() << "\n"
      << "v3d num accs introduced            : " << m_v3d_driver.numAccs() << "\n";
#endif  // QPU_MODE

  return ret;
}


void KernelBase::dump_compile_data(bool output_for_vc4, char const *filename) {
	select_driver(output_for_vc4).dump_compile_data(filename);
}

}  // namespace V3DLib
