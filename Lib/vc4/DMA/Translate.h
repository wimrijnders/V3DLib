#ifndef _LIB_VC4_DMA_TRANSLATE_H
#define _LIB_VC4_DMA_TRANSLATE_H
#include "Common/Seq.h"
#include "Source/Stmt.h"
#include "Target/Syntax.h"

namespace V3DLib {

namespace vc4 {

bool translate_stmt(Instr::List &seq, Stmt::Ptr s);
Seq<Instr> StoreRequest(Var addr_var, Var data_var, bool wait = false);

}  // namespace vc4
}  // namespace V3DLib

#endif  // _LIB_VC4_DMA_TRANSLATE_H
