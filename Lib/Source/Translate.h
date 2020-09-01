#ifndef _QPULIB_TRANSLATE_H_
#define _QPULIB_TRANSLATE_H_

#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace QPULib {

void insertEndCode(Seq<Instr>* seq);
void translateStmt(Seq<Instr>* seq, Stmt* s);

//
// Following Exposed for Sourcetranslate()'s
//
Reg srcReg(Var v);
void varAssign(Seq<Instr>* seq, AssignCond cond, Var v, Expr* expr);

}  // namespace QPULib

#endif  // _QPULIB_TRANSLATE_H_
