#ifndef _V3DLIB_INTERPRETER_H_
#define _V3DLIB_INTERPRETER_H_
#include <stdint.h>

namespace V3DLib {

class Stmt;
class BufferObject;

template<typename T>
class Seq;

/**
 * The interpreter works in a similar way to the emulator.  The
 * difference is that the former operates on source code and the
 * latter on target code.
 */
void interpreter(
	int numCores,               // Number of cores active
	Stmt* stmt,                 // Source code
	int numVars,                // Max var id used in source
	Seq<int32_t> &uniforms,     // Kernel parameters
	BufferObject &heap,
	Seq<char>* output = nullptr // Output from print statements (if NULL, stdout is used)
);

}  // namespace V3DLib

#endif  // _V3DLIB_INTERPRETER_H_
