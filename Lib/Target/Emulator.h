#ifndef _V3DLIB_TARGET_EMULATOR_H_
#define _V3DLIB_TARGET_EMULATOR_H_
#include "instr/Instr.h"

namespace V3DLib {

class BufferObject;

// Emulator
void emulate(
  int numQPUs,                 // Number of QPUs active
  Instr::List &instrs,         // Instruction sequence
  int maxReg,                  // Max reg id used
  Seq<int32_t> &uniforms,      // Kernel parameters
  BufferObject &heap
);

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_EMULATOR_H_
