#ifndef _V3DLIB_SOURCE_TRANSLATE_H_
#define _V3DLIB_SOURCE_TRANSLATE_H_
#include "Common/Seq.h"
#include "Source/Stmt.h"
#include "Target/Syntax.h"

namespace V3DLib {

void translate_stmt(Seq<Instr> &seq, Stmt::Ptr s);
void insertInitBlock(Seq<Instr> &code);
void loadStorePass(Seq<Instr> &instrs);

//
// Following exposed for source translates.
//
Seq<Instr> varAssign(AssignCond cond, Var v, Expr::Ptr expr);
Seq<Instr> varAssign(Var v, Expr::Ptr expr);
Expr::Ptr putInVar(Seq<Instr>* seq, Expr::Ptr e);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_TRANSLATE_H_
