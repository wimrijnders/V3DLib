#ifndef _V3DLIB_KERNEL_H_
#define _V3DLIB_KERNEL_H_
#include <tuple>
#include <algorithm>  // std::move
#include "Source/Int.h"
#include "Source/Complex.h"
#include "Source/Interpreter.h"
#include "Target/Emulator.h"
#include "Common/SharedArray.h"
#include "v3d/Invoke.h"
#include "vc4/vc4.h"
#include "Support/Platform.h"
#include "Support/assign.h"
#include "vc4/KernelDriver.h"
#include "v3d/KernelDriver.h"

namespace V3DLib {

// ============================================================================
// Parameter passing
// ============================================================================

template <typename... ts> inline void nothing(ts... args) {}


template <typename T, typename t> inline bool passParam(Seq<int32_t>* uniforms, t x) {
  return T::passParam(uniforms, x);
}


/**
 * Grumbl still need special override for 2D shared array.
 * Sort of patched this, will sort it out another time.
 *
 * You can not possibly have any idea how long it took me * to implement and use this correctly.
 * Even so, I'm probably doing it wrong.
 */
template <>
inline bool passParam< Float::Ptr, Shared2DArray<float>* > (Seq<int32_t>* uniforms, Shared2DArray<float> *p) {
  return Float::Ptr::passParam(uniforms, &((BaseSharedArray const &) p->get_parent()));
}


// ============================================================================
// Kernels
// ============================================================================

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
class KernelBase {
public:
  KernelBase() {}
  KernelBase(KernelBase &&k) = default;

  void pretty(bool output_for_vc4, const char *filename = nullptr, bool output_qpu_code = true);

  KernelBase &setNumQPUs(int n)       { numQPUs = n;           return *this; }

  void emu();
  void interpret();
  void call();
#ifdef QPU_MODE
  void qpu();
#endif  // QPU_MODE


protected:
  int numQPUs = 1;                 // Number of QPUs to run on
  Seq<int32_t> uniforms;           // Parameters to be passed to kernel
  vc4::KernelDriver m_vc4_driver;  // Always required for emulator
  int numVars;                     // The number of variables in the source code

#ifdef QPU_MODE
  v3d::KernelDriver m_v3d_driver;
#endif
};


/**
 * -------------------------
 * NOTES
 * =====
 *
 * 1. A kernel is parameterised by a list of QPU types 'ts' representing
 *    the types of the parameters that the kernel takes.
 *
 *    The kernel constructor takes a function with parameters of QPU
 *    types 'ts'.  It applies the function to constuct an AST.
 */
template <typename... ts> struct Kernel : public KernelBase {
  using KernelFunction = void (*)(ts... params);

  // Construct an argument of QPU type 't'.
  template <typename T> inline T mkArg() { return T::mkArg(); }

public:
  Kernel(Kernel const &k) = delete;
  Kernel(Kernel &&k) = default;

  /**
   * Construct kernel out of C++ function
   */
  Kernel(KernelFunction f, bool vc4_only = false) {
    {
      m_vc4_driver.compile_init();

      auto args = std::make_tuple(mkArg<ts>()...);

      // Construct the AST for vc4
      apply(f, args);

      m_vc4_driver.compile();

      // Remember the number of variables used - for emulator/interpreter
      numVars = getFreshVarCount();
    }

#ifdef QPU_MODE
    if (!vc4_only) {
      m_v3d_driver.compile_init();

      // Construct the AST for v3d
      f(mkArg<ts>()...);

      m_v3d_driver.compile();
    }
#endif  // QPU_MODE
  }


  /**
   * Load uniform values.
   *
   * Pass params, checking arguments types us against parameter types ts.
   */
  template <typename... us>
  Kernel &load(us... args) {
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args)...);

    return *this;
  }
};

// Initialiser

template <typename... ts>
Kernel<ts...> compile(void (*f)(ts... params), bool vc4_only = false) {
  Kernel<ts...> k(f, vc4_only);
  return std::move(k);
}

}  // namespace V3DLib

#endif  // _V3DLIB_KERNEL_H_
