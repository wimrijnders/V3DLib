#ifndef _V3DLIB_VC4_DMA_LOADSTORE_H_
#define _V3DLIB_VC4_DMA_LOADSTORE_H_
#include "Source/Stmt.h"
#include "Target/Syntax.h"

namespace V3DLib {
namespace DMA {

Instr::List varassign_deref_var(Var &v, Expr &e);
bool translate_stmt(Instr::List &seq, int in_tag, Stmt &s);
Instr::List StoreRequest(Var addr_var, Var data_var);

}  // namespace DMA
}  // namespace V3DLib

#endif  // _V3DLIB_VC4_DMA_LOADSTORE_H_
