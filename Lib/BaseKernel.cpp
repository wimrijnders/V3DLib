#include "BaseKernel.h"
#include "Support/basics.h"
#include "Source/Interpreter.h"
#include "Target/Emulator.h"
#include "Target/Pretty.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

BaseKernel::BaseKernel() {}

bool BaseKernel::has_vc4() const { return m_vc4_driver.get() != nullptr; }
bool BaseKernel::has_v3d() const { return m_v3d_driver.get() != nullptr; }


V3DLib::KernelDriver &BaseKernel::vc4() {
  assertq(has_vc4(), "vc4 driver not enabled for this kernel");
  return *m_vc4_driver;
}


V3DLib::KernelDriver const &BaseKernel::vc4() const {
  assertq(has_vc4(), "vc4 driver not enabled for this kernel");
  return *m_vc4_driver;
}


V3DLib::KernelDriver &BaseKernel::v3d() {
  assertq(has_v3d(), "v3d driver not enabled for this kernel", true);
  return *m_v3d_driver;
}


V3DLib::KernelDriver const &BaseKernel::v3d() const {
  assertq(has_v3d(), "v3d driver not enabled for this kernel", true);
  return *m_v3d_driver;
}


void BaseKernel::compile_init(bool do_vc4) {
  if (do_vc4) {
    assert(!has_vc4());
    m_vc4_driver.reset(new vc4::KernelDriver);
    vc4().init_compile();
    Platform::compiling_for_vc4(true);
  } else {
    assert(!has_v3d());
    m_v3d_driver.reset(new v3d::KernelDriver);
    v3d().init_compile();
    Platform::compiling_for_vc4(false);
  }
}


bool BaseKernel::has_errors() const {
 return (has_vc4() && vc4().has_errors()) || (has_v3d() && v3d().has_errors());
}


void BaseKernel::pretty(bool output_for_vc4, const char *filename, bool output_qpu_code) {
  if (output_for_vc4) {
    vc4().pretty(filename, output_qpu_code);
  } else {
    v3d().pretty(filename, output_qpu_code);
  }
}


/**
 * Invoke the emulator
 *
 * The emulator runs vc4 code.
 */
void BaseKernel::emu() {
  if (vc4().has_errors()) {
    warning("Not running on emulator, there were errors during compile.");
    return;
  }

  assert(uniforms.size() != 0);
  emulate(numQPUs, vc4().targetCode(), vc4().numVars(), uniforms, getBufferObject());
}


/**
 * Invoke the interpreter
 */
void BaseKernel::interpret() {
  if (vc4().has_errors()) {
    warning("Not running interpreter, there were errors during compile.");
    return;
  }

  assert(uniforms.size() != 0);
  interpreter(numQPUs, vc4().sourceCode(), vc4().numVars(), uniforms, getBufferObject());
}


#ifdef QPU_MODE
/**
 * Invoke kernel on physical QPU hardware
 */
void BaseKernel::qpu() {
  if (Platform::has_vc4()) {
    vc4().invoke(numQPUs, uniforms);
  } else {
    v3d().invoke(numQPUs, uniforms);
  }
}
#endif  // QPU_MODE


/**
 * Invoke the kernel
 */
void BaseKernel::call() {
#ifdef QPU_MODE
  if (Platform::use_main_memory()) {
    warning("Main memory selected in QPU mode, running on emulator instead of QPU.");
    emu();
  } else {
    qpu();
  }
#else
  emu();
#endif
};


std::string BaseKernel::compile_info() const {
  std::string ret;

  ret << "\n"
      << "Compile info\n"
      << "============\n";

  if (!has_vc4() && !has_v3d()) {
    ret << "No kernel drivers enabled\n\n";
  } else {
    if (has_vc4()) {
      ret << "vc4:\n"
          << vc4().compile_info() << "\n\n";
    }

    if (has_v3d()) {
      ret << "vc4:\n"
          << v3d().compile_info() << "\n\n";
    }
  }

  return ret;
}


void BaseKernel::dump_compile_data(bool output_for_vc4, char const *filename) {
  if (output_for_vc4) {
    vc4().dump_compile_data(filename);
  } else {
    v3d().dump_compile_data(filename);
  }
}


std::string BaseKernel::get_errors() const {
  std::string ret;

  if (has_vc4() && vc4().has_errors()) {
    ret << vc4().get_errors();
  }

  if (has_v3d() && v3d().has_errors()) {
    ret << v3d().get_errors();
  }

  if (ret.empty()) {
    ret = "<No Errors>";
  }

  return ret;
}


int BaseKernel::v3d_kernel_size() const {
  assert(m_v3d_driver.get() != nullptr);
  return m_v3d_driver->kernel_size();
}

}  // namespace V3DLib
