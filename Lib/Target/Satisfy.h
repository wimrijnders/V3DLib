#ifndef _V3DLIB_SATISFY_H_
#define _V3DLIB_SATISFY_H_

#include "Target/Syntax.h"
#include "Target/CFG.h"

namespace V3DLib {

RegTag regFileOf(Reg r);
void satisfy(Seq<Instr>* instrs);

}  // namespace V3DLib

#endif  // _V3DLIB_SATISFY_H_
