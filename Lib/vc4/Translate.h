#ifndef _LIB_VC4_TRANSLATE_H
#define _LIB_VC4_TRANSLATE_H
#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace QPULib {
namespace vc4 {

bool stmt(Seq<Instr>* seq, Stmt* s);

}  // namespace vc4
}  // namespace QPULib

#endif  // _LIB_VC4_TRANSLATE_H
