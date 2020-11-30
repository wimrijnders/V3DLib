#ifndef _V3DLIB_SOURCETRANSLATE_H_
#define _V3DLIB_SOURCETRANSLATE_H_
#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"
#include "Target/CFG.h"

namespace V3DLib {

class Stmt;

class ISourceTranslate {
public:
	virtual ~ISourceTranslate() {}

	/**
	 * TODO: return value currently not used, remove?
	 *
	 * @return true if handled, false otherwise
	 */
	virtual bool deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr::Ptr rhs) = 0;
	virtual void varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) = 0;
	virtual void regAlloc(CFG* cfg, Seq<Instr>* instrs) = 0;
	virtual bool stmt(Seq<Instr>& seq, Stmt* s) = 0;
};

ISourceTranslate &getSourceTranslate();

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCETRANSLATE_H_
