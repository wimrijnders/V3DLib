#ifndef _V3DLIB_VC4_DMA_LOADSTORE_H_
#define _V3DLIB_VC4_DMA_LOADSTORE_H_
#include "Source/Stmt.h"
#include "Target/instr/Instr.h"

namespace V3DLib {
namespace DMA {

Instr::List loadRequest(Var &dst, Expr &e);
Instr::List storeRequest(Var dst_addr, Var src);
bool translate_stmt(Instr::List &seq, int in_tag, Stmt &s);

}  // namespace DMA
}  // namespace V3DLib

#endif  // _V3DLIB_VC4_DMA_LOADSTORE_H_
