#ifndef _V3DLIB_SUBST_H_
#define _V3DLIB_SUBST_H_
#include "Target/Syntax.h"

namespace V3DLib {

void renameDest(Instr* instr, RegTag vt, RegId v, RegTag wt, RegId w);
void renameUses(Instr* instr, RegTag vt, RegId v, RegTag wt, RegId w);
void substRegTag(Instr* instr, RegTag vt, RegTag wt);

}  // namespace V3DLib

#endif  // _V3DLIB_SUBST_H_
