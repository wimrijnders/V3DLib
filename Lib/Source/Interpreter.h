#ifndef _QPULIB_INTERPRETER_H_
#define _QPULIB_INTERPRETER_H_
#include <stdint.h>
#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Common/BufferObject.h"

namespace QPULib {

/**
 * The interpreter works in a similar way to the emulator.  The
 * difference is that the former operates on source code and the
 * latter on target code.  We reuse a number of concepts of the
 * emulator in the interpreter.
 */
void interpreter(
	int numCores,            // Number of cores active
	Stmt* stmt,              // Source code
	int numVars,             // Max var id used in source
	Seq<int32_t>* uniforms,  // Kernel parameters
	BufferObject &heap,
	Seq<char>* output = NULL // Output from print statements (if NULL, stdout is used)
);

}  // namespace QPULib

#endif  // _QPULIB_INTERPRETER_H_
