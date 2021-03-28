#ifndef _V3DLIB_SOURCE_TRANSLATE_H_
#define _V3DLIB_SOURCE_TRANSLATE_H_
#include "Source/Stmt.h"
#include "Target/instr/Instr.h"

namespace V3DLib {

void translate_stmt(Instr::List &seq, Stmt::Ptr s);
void insertInitBlock(Instr::List &code);
void loadStorePass(Instr::List &instrs);

//
// Following exposed for source translates.
//
Instr::List varAssign(AssignCond cond, Var v, Expr::Ptr expr);
Instr::List varAssign(Var v, Expr::Ptr expr);
Expr::Ptr putInVar(Instr::List *seq, Expr::Ptr e);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_TRANSLATE_H_
