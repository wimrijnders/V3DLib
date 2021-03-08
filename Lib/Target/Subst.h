#ifndef _V3DLIB_SUBST_H_
#define _V3DLIB_SUBST_H_
#include "Target/instr/Instr.h"

namespace V3DLib {

void renameDest(Instr &instr, Reg const &current, Reg const &replace_with);
void renameUses(Instr &instr, Reg const &current, Reg const &replace_with);
void substRegTag(Instr* instr, RegTag vt, RegTag wt);

}  // namespace V3DLib

#endif  // _V3DLIB_SUBST_H_
