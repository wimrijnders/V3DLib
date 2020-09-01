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
};

ISourceTranslate &getSourceTranslate();
void set_compiling_for_vc4(bool val);
bool compiling_for_vc4();

}  // namespace QPULib

#endif  // _QPULIB_SOURCETRANSLATE_H_
