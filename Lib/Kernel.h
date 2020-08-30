#ifndef _QPULIB_KERNEL_H_
#define _QPULIB_KERNEL_H_
#include "Source/Interpreter.h"
#include "Target/Emulator.h"
#include "Common/SharedArray.h"
#include "v3d/Invoke.h"
#include "vc4/vc4.h"
#include "Support/Platform.h"
#include  "vc4/KernelDriver.h"
#include  "v3d/KernelDriver.h"
#include  "SourceTranslate.h"  // set_compiling_for_vc4()

namespace QPULib {


// ============================================================================
// Modes of operation
// ============================================================================

/**
 * The interpreter and emulator are always available. However, these run only
 * vc4 code. These will run on any architecture.
 *
 * The compile-time option `-D QPU_MODE` enables the execution of code on
 * the VideoCore. This will work only on the Rasberry Pi.
 * This macro is optional.
 *
 * We will define 'emulation mode' as a build without QPU_MODE defined.
 *
 * The memory pool management depend on this macro and on which 
 * Pi platform is run:
 *
 * 1. Pure emulation mode (QPU_MODE not defined): 
 *    Main memory is used.
 * 2. QPU-mode on Pi4 (and lateri, v3d):
 *    Uses the `v3d` memory allocation scheme, also for
 *    emulation mode. This addresses the gpu-device for
 *    allocating.
 * 3. QPU-mode before Pi4 (vc4):
 *    Uses the `v3d` memory allocation scheme, also for
 *    emulation mode. A ox interface to the
 *    vc4 is used to allocate memory.
 *
 * The memory pool implementations are in source files
 * called `BufferObject.cpp` (Under `Target`, `v3d` and `vc4`).
 */

// The 'Kernel' class provides various ways to invoke a kernel:
//
//   * qpu(...)        invoke kernel on physical QPUs
//                     (only available in QPU_MODE)
//   * emulate(...)    invoke kernel using target code emulator
//   * interpret(...)  invoke kernel using source code interpreter
//   * call(...)       in emulation mode, same as emulate(...)
//                     with QPU_MODE, same as qpu(...)
//
// Emulation mode calls are provided for doing equivalence
// testing between the physical QPU and the QPU emulator.  However,
// emulation mode introduces a performance penalty and should be used
// only for testing and debugging purposes.


// ============================================================================
// Kernel arguments
// ============================================================================

// Construct an argument of QPU type 't'.

template <typename t> inline t mkArg();

template <> inline Int mkArg<Int>() {
  Int x;
  x = getUniformInt();
  return x;
}

template <> inline Float mkArg<Float>() {
  Float x;
  x = getUniformFloat();
  return x;
}

template <> inline Ptr<Int> mkArg< Ptr<Int> >() {
  Ptr<Int> x;
  x = getUniformPtr<Int>();
  return x;
}

template <> inline Ptr<Float> mkArg< Ptr<Float> >() {
  Ptr<Float> x;
  x = getUniformPtr<Float>();
  return x;
}

// ============================================================================
// Parameter passing
// ============================================================================

template <typename... ts> inline void nothing(ts... args) {}

// Pass argument of ARM type 'u' as parameter of QPU type 't'.

template <typename t, typename u>
inline bool passParam(Seq<int32_t>* uniforms, u x);

// Pass an int
template <> inline bool passParam<Int, int>
  (Seq<int32_t>* uniforms, int x)
{
  uniforms->append((int32_t) x);
  return true;
}

// Pass a float
template <> inline bool passParam<Float, float>
	(Seq<int32_t>* uniforms, float x)
{
  int32_t* bits = (int32_t*) &x;
  uniforms->append(*bits);
  return true;
}

// Pass a SharedArray<int>*
template <> inline bool passParam< Ptr<Int>, SharedArray<int>* >
  (Seq<int32_t>* uniforms, SharedArray<int>* p)
{
  uniforms->append(p->getAddress());
  return true;
}


// Pass a SharedArray<float>*
template <> inline bool passParam< Ptr<Float>, SharedArray<float>* >
  (Seq<int32_t>* uniforms, SharedArray<float>* p)
{
  uniforms->append(p->getAddress());
  return true;
}


// ============================================================================
// Kernels
// ============================================================================

class KernelBase {
public:
	void pretty(const char *filename = nullptr);

  // Set number of QPUs to use
  void setNumQPUs(int n) {
    numQPUs = n;
  }

	int maxQPUs() {
		if (Platform::instance().has_vc4) {
			return 12;
		} else {
			return 8;
		}
	}

protected:
	int numQPUs = 1;                 // Number of QPUs to run on
	Seq<int32_t> uniforms;           // Parameters to be passed to kernel
	vc4::KernelDriver m_vc4_driver;  // Always required for emulator

#ifdef QPU_MODE
	v3d::KernelDriver m_v3d_driver;
#endif

	void init_compile();
	void invoke_qpu(QPULib::KernelDriver &kernel_driver);
};


/**
 *
 * ----------------------------------------------------------------------------
 * NOTES
 * ====
 *
 * * A kernel is parameterised by a list of QPU types 'ts' representing
 *   the types of the parameters that the kernel takes.
 *
 *   The kernel constructor takes a function with parameters of QPU
 *   types 'ts'.  It applies the function to constuct an AST.
 *
 *
 * * The code generation for v3d and vc4 is diverging, even at the level of
 *   source code. To handle this, the code generation for both cases is 
 *   isolated in 'kernel drivers'. This encapsulates the differences
 *   for the kernel.
 *
 *   At time of writing (20200818), for the source code it is notably the end
 *   program sequence (see `kernelFinish()`).
 *
 *   The interpreter and emulator, however, work with vc4 code. For this reason
 *   it is necessary to have the vc4 kernel driver in use in all build cases.
 */
template <typename... ts> struct Kernel : public KernelBase {
	using KernelFunction = void (*)(ts... params);

public:

	/**
   * Construct kernel out of C++ function
	 */
  Kernel(KernelFunction f) {
		{
			init_compile();
			set_compiling_for_vc4(true);

	    // Construct the AST for vc4
	    f(mkArg<ts>()...);
			m_vc4_driver.compile();

    	// Remember the number of variables used - for emulator/interpreter
	    numVars = getFreshVarCount();
		}

#ifdef QPU_MODE
		{
			init_compile();
			set_compiling_for_vc4(false);

	    // Construct the AST for v3d
	    f(mkArg<ts>()...);
			m_v3d_driver.compile();
		}
#endif  // QPU_MODE
  }

  template <typename... us> void emu(us... args) {
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args)...);

		// Emulator runs the vc4 code
    emulate(numQPUs, &m_vc4_driver.targetCode(), numVars, &uniforms, getBufferObject());
  }

  // Invoke the interpreter
  template <typename... us> void interpret(us... args) {
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args)...);

		// Interpreter runs the vc4 code
    interpreter(numQPUs, m_vc4_driver.sourceCode(), numVars, &uniforms, getBufferObject());
  }


#ifdef QPU_MODE
  // Invoke kernel on physical QPU hardware
  template <typename... us> void qpu(us... args) {
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args)...);

		if (Platform::instance().has_vc4) {
			invoke_qpu(m_vc4_driver);
		} else {
			invoke_qpu(m_v3d_driver);
		}
  }
#endif  // QPU_MODE

 
  // Invoke the kernel
  template <typename... us> void call(us... args) {
#ifdef EMULATION_MODE
		emu(args...);
#else
#ifdef QPU_MODE
    qpu(args...);
#endif
#endif
  };

  // Overload function application operator
  template <typename... us> void operator()(us... args) {
    call(args...);
  }


private:
	int numVars;            // The number of variables in the source code
};

// Initialiser

template <typename... ts> Kernel<ts...> compile(void (*f)(ts... params))
{
  Kernel<ts...> k(f);
  return k;
}

}  // namespace QPULib

#endif  // _QPULIB_KERNEL_H_
