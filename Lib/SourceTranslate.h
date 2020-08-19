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
};

ISourceTranslate &getSourceTranslate();

}  // namespace QPULib

#endif  // _QPULIB_SOURCETRANSLATE_H_
