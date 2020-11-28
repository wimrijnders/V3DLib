#ifndef _LIB_VC4_TRANSLATE_H
#define _LIB_VC4_TRANSLATE_H
#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace V3DLib {

class Stmt;

namespace vc4 {


bool stmt(Seq<Instr>* seq, Stmt* s);
Seq<Instr> StoreRequest(Var addr_var, Var data_var, bool wait = false);

}  // namespace vc4
}  // namespace V3DLib

#endif  // _LIB_VC4_TRANSLATE_H
