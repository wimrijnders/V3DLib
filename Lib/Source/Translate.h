#ifndef _V3DLIB_TRANSLATE_H_
#define _V3DLIB_TRANSLATE_H_
#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace V3DLib {

void translateStmt(Seq<Instr> &seq, Stmt *s);
void loadStorePass(Seq<Instr> &instrs);

//
// Following exposed for source translates.
//
void varAssign(Seq<Instr>* seq, AssignCond cond, Var v, ExprPtr expr);
void varAssign(Seq<Instr>* seq, Var v, ExprPtr expr);
ExprPtr putInVar(Seq<Instr>* seq, ExprPtr e);

}  // namespace V3DLib

#endif  // _V3DLIB_TRANSLATE_H_
