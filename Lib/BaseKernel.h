#ifndef _V3DLIB_BASEKERNEL_H_
#define _V3DLIB_BASEKERNEL_H_
#include <memory>
#include "vc4/KernelDriver.h"
#include "v3d/KernelDriver.h"

namespace V3DLib {


/**
 * -------------------------
 * NOTES
 * =====
 *
 * 1. A kernel can be invoked with the following methods of `Kernel`:
 *
 *     - interpret(...)  - run on source code interpreter
 *     - emu(...)        - run on the target code emulator (`vc4` code only)
 *     - qpu(...)        - run on physical QPUs (only when QPU_MODE enabled))
 *     - call(...)       - depending on QPU_MODE, call `qpu()` or `emu()`
 *                      This is useful for cross-platform compatibility
 *
 *    The interpreter and emulator are useful for development/debugging and 
 *    for equivalence testing for the hardware QPU.
 *
 *    Just keep in mind that the interpreter and emulator are really slow
 *    in comparison to the hardware.
 *
 *
 * 2. The interpreter and emulator will run on any architecture.
 *
 *    The compile-time option `-D QPU_MODE` enables the execution of code on
 *    the physical VideoCore. This will work only on the Rasberry Pi.
 *    This define is disabled in the makefile for non-Pi platforms.
 *
 *    There are three possible shared memory allocation schemes:
 *
 *    1. Using main memory - selectable in code, will not work for hardware GPU
 *    2. `vc4` memory allocation - used when communicating with hardware GPU on
 *       all Pi's prior to Pi4
 *    3. `v3d` memory allocation - used when communicating with hardware GPU on the Pi4 
 *
 *    The memory pool implementations are in source files
 *    called `BufferObject.cpp` (Under `Target`, `v3d` and `vc4`).
 *
 *
 * 3. The code generation for v3d and vc4 has diverged, even at the level of
 *    source code. To handle this, the code generation for both cases is 
 *    isolated in 'kernel drivers'. This encapsulates the differences
 *    for the kernel.
 *
 *    Because the interpreter and emulator work with vc4 code,
 *    the vc4 kernel driver is always used, even if only assembling for v3d.
 */
class BaseKernel {
public:
  BaseKernel();
  BaseKernel(BaseKernel &&k) = default;

  bool has_vc4() const;
  bool has_v3d() const;
  V3DLib::KernelDriver &vc4();
  V3DLib::KernelDriver &v3d();
  V3DLib::KernelDriver const &vc4() const;
  V3DLib::KernelDriver const &v3d() const;

  void compile_init(bool do_vc4);
  void pretty(bool output_for_vc4, const char *filename = nullptr, bool output_qpu_code = true);

  BaseKernel &setNumQPUs(int n) { numQPUs = n; return *this; }

  void emu();
  void interpret();
  void call();
#ifdef QPU_MODE
  void qpu();
#endif  // QPU_MODE

  std::string compile_info() const;
  void dump_compile_data(bool output_for_vc4, char const *filename);
  bool has_errors() const;
  std::string get_errors() const;

//protected:
//  void encode();

protected:
  int numQPUs = 1;                 // Number of QPUs to run on
  IntList uniforms;                // Parameters to be passed to kernel

  // Defined as unique pointers so that they easily survive the std::move
  // (There are other reasons but this is the main one)
  std::unique_ptr<vc4::KernelDriver> m_vc4_driver;
  std::unique_ptr<v3d::KernelDriver> m_v3d_driver;
};


}  // namespace V3DLib

#endif  // _V3DLIB_BASEKERNEL_H_
