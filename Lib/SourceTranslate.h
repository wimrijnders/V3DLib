#ifndef _V3DLIB_SOURCETRANSLATE_H_
#define _V3DLIB_SOURCETRANSLATE_H_
#include "Common/Seq.h"
#include "Source/Stmt.h"
#include "Target/Syntax.h"
#include "Target/CFG.h"

namespace V3DLib {

class ISourceTranslate {
public:
	virtual ~ISourceTranslate() {}

	/**
	 * TODO: return value currently not used, remove?
	 *
	 * @return true if handled, false otherwise
	 */
	virtual Instr::List deref_var_var(Var lhs, Var rhs) = 0;
	virtual void varassign_deref_var(Instr::List *seq, Var &v, Expr &e) = 0;
	virtual void regAlloc(CFG* cfg, Instr::List *instrs) = 0;
	virtual bool stmt(Instr::List &seq, Stmt::Ptr s) = 0;
};

ISourceTranslate &getSourceTranslate();

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCETRANSLATE_H_
