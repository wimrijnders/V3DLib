#ifndef _V3DLIB_SATISFY_H_
#define _V3DLIB_SATISFY_H_
#include "Target/instr/Instr.h"

namespace V3DLib {

RegTag regFileOf(Reg r);
void satisfy(Instr::List &instrs);

}  // namespace V3DLib

#endif  // _V3DLIB_SATISFY_H_
