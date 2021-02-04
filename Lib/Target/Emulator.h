#ifndef _V3DLIB_TARGET_EMULATOR_H_
#define _V3DLIB_TARGET_EMULATOR_H_
#include <stdint.h>

namespace V3DLib {

class Instr;
class BufferObject;

template<typename T>
class Seq;

// Emulator
void emulate(
  int numQPUs,                 // Number of QPUs active
  Seq<Instr>* instrs,          // Instruction sequence
  int maxReg,                  // Max reg id used
  Seq<int32_t> &uniforms,      // Kernel parameters
  BufferObject &heap,
  Seq<char>* output = nullptr  // Output from print statements (if NULL, stdout is used)
);

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_EMULATOR_H_
