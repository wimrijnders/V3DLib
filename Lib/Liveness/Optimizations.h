#ifndef _V3DLIB_LIVENESS_OPTIMIZATIONS_H_
#define _V3DLIB_LIVENESS_OPTIMIZATIONS_H_
#include "Target/instr/Instr.h"

namespace V3DLib {

class Liveness;

bool combineImmediates(Liveness &live, Instr::List &instrs);
int introduceAccum(Liveness &live, Instr::List &instrs);

}  // namespace V3DLib

#endif  // _V3DLIB_LIVENESS_OPTIMIZATIONS_H_
