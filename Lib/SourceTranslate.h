#ifndef _QPULIB_SOURCETRANSLATE_H_
#define _QPULIB_SOURCETRANSLATE_H_
#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"

namespace QPULib {

class ISourceTranslate {
public:
	virtual ~ISourceTranslate() {}

	/**
	 * TODO: return value currently not used, remove?
	 *
	 * @return true if handled, false otherwise
	 */
	virtual bool deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr *rhs) = 0;

	virtual void setupVPMWriteStmt(Seq<Instr>* seq, Stmt *s) = 0;
	virtual void storeRequest(Seq<Instr>* seq, Expr* data, Expr* addr) = 0;
	virtual void varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) = 0;

	virtual void regalloc_determine_regfileAB(Seq<Instr> *instrs, int *prefA, int *prefB, int n) = 0;
};

ISourceTranslate &getSourceTranslate();
void set_compiling_for_vc4(bool val);
bool compiling_for_vc4();

}  // namespace QPULib

#endif  // _QPULIB_SOURCETRANSLATE_H_
