#ifndef _V3DLIB_TRANSLATE_H_
#define _V3DLIB_TRANSLATE_H_
#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace V3DLib {

class Stmt;

void translateStmt(Seq<Instr> &seq, Stmt *s);
void loadStorePass(Seq<Instr> &instrs);

//
// Following exposed for source translates.
//
void varAssign(Seq<Instr>* seq, AssignCond cond, Var v, Expr::Ptr expr);
void varAssign(Seq<Instr>* seq, Var v, Expr::Ptr expr);
Expr::Ptr putInVar(Seq<Instr>* seq, Expr::Ptr e);

}  // namespace V3DLib

#endif  // _V3DLIB_TRANSLATE_H_
