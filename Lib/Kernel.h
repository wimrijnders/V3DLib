#ifndef _QPULIB_KERNEL_H_
#define _QPULIB_KERNEL_H_
#include "Source/Interpreter.h"
#include "Target/Emulator.h"
#include "Common/SharedArray.h"
#include "v3d/Invoke.h"
#include "vc4/vc4.h"
//#include "Source/Pretty.h"
#include "Target/Pretty.h"
#include "Support/Platform.h"
#include  "vc4/KernelDriver.h"
#include  "v3d/KernelDriver.h"

namespace QPULib {


// ============================================================================
// Modes of operation
// ============================================================================

// Two important compile-time macros are EMULATION_MODE and QPU_MODE.
// With -D EMULATION_MODE, QPULib can be compiled for any architecture.
// With -D QPU_MODE, QPULib can be compiled only for the Raspberry Pi.
// At least one of these macros must be defined.

// IN EMULATION_MODE a memory pool is used for allocating data that
// can be read by kernels.  Otherwise, a mailbox interface to the
// vc4 is used to allocate memory.  In both cases, see
// 'vc4/SharedArray.h'.

// The 'Kernel' class provides various ways to invoke a kernel:
//
//   * qpu(...)        invoke kernel on physical QPUs
//                     (only available in QPU_MODE)
//   * emulate(...)    invoke kernel using target code emulator
//                     (only available in EMULATION_MODE)
//   * interpret(...)  invoke kernel using source code interpreter
//                     (only available in EMULATION_MODE)
//   * call(...)       in EMULATION_MODE, same as emulate(...)
//                     in QPU_MODE, same as qpu(...)
//                     in EMULATION_MODE *and* QPU_MODE, same as emulate(...)

// Notice it is OK to compile with both -D EMULATION_MODE *and*
// -D QPU_MODE.
//
// --------------------------------------------------------------------
// **UPDATE 20200616**: Using both flags will compile, but
//                      leads to segmentation Faults.
//
// A possible reason for this is that emulation and QPU require
// differing implementations of `SharedArray<>`, which can not be used
// both at the same time. Hence, the QPU implementation was used, which
// might (far-fetched) have something to do with the segmentation faults.
// --------------------------------------------------------------------
//
// This feature is provided for doing equivalance
// testing between the physical QPU and the QPU emulator.  However,
// EMULATION_MODE introduces a performance penalty and should be used
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

template <typename t, typename u> inline bool
  passParam(Seq<int32_t>* uniforms, u x, BufferType buftype);

// Pass an int
template <> inline bool passParam<Int, int>
  (Seq<int32_t>* uniforms, int x, BufferType buftype)
{
  uniforms->append((int32_t) x);
  return true;
}

// Pass a float
template <> inline bool passParam<Float, float>
  (Seq<int32_t>* uniforms, float x, BufferType buftype)
{
  int32_t* bits = (int32_t*) &x;
  uniforms->append(*bits);
  return true;
}

// Pass a SharedArray<int>*
template <> inline bool passParam< Ptr<Int>, SharedArray<int>* >
  (Seq<int32_t>* uniforms, SharedArray<int>* p, BufferType buftype)
{
	//p->setType(buftype);
  uniforms->append(p->getAddress());
  return true;
}

// Pass a SharedArray<int*>*
template <> inline bool passParam< Ptr<Ptr<Int>>, SharedArray<int*>* >
  (Seq<int32_t>* uniforms, SharedArray<int*>* p, BufferType buftype)
{
	//p->setType(buftype);
  uniforms->append(p->getAddress());
  return true;
}

// Pass a SharedArray<float>*
template <> inline bool passParam< Ptr<Float>, SharedArray<float>* >
  (Seq<int32_t>* uniforms, SharedArray<float>* p, BufferType buftype)
{
	//p->setType(buftype);
  uniforms->append(p->getAddress());
  return true;
}

// Pass a SharedArray<float*>*
template <> inline bool passParam< Ptr<Ptr<Float>>, SharedArray<float*>* >
  (Seq<int32_t>* uniforms, SharedArray<float*>* p, BufferType buftype)
{
	//p->setType(buftype);
  uniforms->append(p->getAddress());
  return true;
}


// ============================================================================
// Functions on kernels
// ============================================================================

// Compile a kernel
void compileKernel(Seq<Instr>* targetCode, Stmt* s);

// ============================================================================
// Kernels
// ============================================================================

// A kernel is parameterised by a list of QPU types 'ts' representing
// the types of the parameters that the kernel takes.

// The kernel constructor takes a function with parameters of QPU
// types 'ts'.  It applies the function to constuct an AST.

template <typename... ts> struct Kernel {
	using KernelFunction = void (*)(ts... params);

public:
  ~Kernel() {}

  // AST representing the target code
  Seq<Instr> targetCode;

  // Parameters to be passed to kernel
  Seq<int32_t> uniforms;

  // The number of variables in the source code
  int numVars;

  // Number of QPUs to run on
  int numQPUs;

  // Construct kernel out of C++ function
  Kernel(KernelFunction f) {
    numQPUs = 1;

    controlStack.clear();
    stmtStack.clear();
    stmtStack.push(mkSkip());
    resetFreshVarGen();
    resetFreshLabelGen();

    // Reserved general-purpose variables
    Int qpuId, qpuCount;
    qpuId = getUniformInt();
    qpuCount = getUniformInt();

    // Construct the AST
    f(mkArg<ts>()...);

		m_vc4_driver.transfer_stack();

    // Compile
    compileKernel(&targetCode, m_vc4_driver.sourceCode());

    // Remember the number of variables used
    numVars = getFreshVarCount();
  }

//#ifdef EMULATION_MODE
  template <typename... us> void emu(us... args) {
breakpoint
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args, BufferType::HeapBuffer)...);

		// NOTE: The emulator is based on the vc4.
		//       This implies that, even though we're running on the v3d, the
		//       target code should be constructed for vc4.
		//
		// TODO: Fix this

    emulate
      ( numQPUs          // Number of QPUs active
      , &targetCode      // Instruction sequence
      , numVars          // Number of vars in source
      , &uniforms        // Kernel parameters
      , NULL             // Use stdout
      );
  }
//#endif

//#ifdef EMULATION_MODE
  // Invoke the interpreter
  template <typename... us> void interpret(us... args) {
breakpoint
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args, BufferType::HeapBuffer)...);

		// NOTE: There are differences in source code generation for vc4 and v3d.
		//       At time of writing (20200818), this is notably the end program
		//       sequence (see `kernelFinish()`).
		//       To be fully consistent with the original, the vc4 source code
		//       should be constructed on the v3d as well. Chances are it will
		//       work regardless.
		// TODO: Investigate if the differences in source code are an issue

    interpreter
      ( numQPUs          // Number of QPUs active
      , m_vc4_driver.sourceCode()       // Source program
      , numVars          // Number of vars in source
      , &uniforms        // Kernel parameters
      , NULL             // Use stdout
      );
  }
//#endif

#ifdef QPU_MODE
  // Invoke kernel on physical QPU hardware
  template <typename... us> void qpu(us... args) {
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();

		if (Platform::instance().has_vc4) {
	    nothing(passParam<ts, us>(&uniforms, args, m_vc4_driver.buffer_type)...);
			invoke_qpu(m_vc4_driver);
		} else {
	    nothing(passParam<ts, us>(&uniforms, args, m_v3d_driver.buffer_type)...);
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

  // Set number of QPUs to use
  void setNumQPUs(int n) {
    numQPUs = n;
  }


  /**
   * @brief Output a human-readable representation of the source and target code.
   *
   * @param filename  if specified, print the output to this file. Otherwise, print to stdout
   */
  void pretty(const char *filename = nullptr) {
    FILE *f = nullptr;

    if (filename == nullptr)
      f = stdout;
    else
    {
      f = fopen(filename, "w");
      if (f == nullptr)
      {
        fprintf(stderr, "ERROR: could not open file '%s' for pretty output\n", filename);
        return;
      }
    }

		m_vc4_driver.pretty(f);  // TODO: Print for v3d as well

#ifdef QPU_MODE
    // Emit target code
    fprintf(f, "Target code\n");
    fprintf(f, "===========\n\n");
    for (int i = 0; i < targetCode.numElems; i++)
    {
      fprintf(f, "%i: ", i);
      QPULib::pretty(f, targetCode.elems[i]);
    }
    fprintf(f, "\n");
    fflush(f);
#endif  // QPU_MODE

    if (filename != nullptr) {
      assert(f != nullptr);
      assert(f != stdout);
      fclose(f);
    }
  }

private:
	vc4::KernelDriver m_vc4_driver;  // Always required for emulator

#ifdef QPU_MODE
	v3d::KernelDriver m_v3d_driver;
#endif

	void invoke_qpu(QPULib::KernelDriver &kernel_driver) {
		kernel_driver.encode(numQPUs, targetCode);
		if (!kernel_driver.handle_errors()) {
    	// Invoke kernel on QPUs
			kernel_driver.invoke(numQPUs, &uniforms);
		}
	}
};

// Initialiser

template <typename... ts> Kernel<ts...> compile(void (*f)(ts... params))
{
  Kernel<ts...> k(f);
  return k;
}

}  // namespace QPULib

#endif  // _QPULIB_KERNEL_H_
