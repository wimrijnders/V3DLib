#ifndef _QPULIB_TRANSLATE_H_
#define _QPULIB_TRANSLATE_H_
#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace QPULib {

void translateStmt(Seq<Instr>* seq, Stmt* s);
void loadStorePass(Seq<Instr>* instrs);

//
// Following exposed for source translates.
//
Reg srcReg(Var v);
Reg dstReg(Var v);
void varAssign(Seq<Instr>* seq, AssignCond cond, Var v, Expr* expr);
void varAssign(Seq<Instr>* seq, Var v, Expr* expr);
Expr* putInVar(Seq<Instr>* seq, Expr* e);

}  // namespace QPULib

#endif  // _QPULIB_TRANSLATE_H_
