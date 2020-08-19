#ifndef _QPULIB_VC4_SOURCETRANSLATE_H_
#define _QPULIB_VC4_SOURCETRANSLATE_H_
#include "../SourceTranslate.h"

namespace QPULib {
namespace vc4 {

class SourceTranslate : public ISourceTranslate {
public:
	bool deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr *rhs) override;
};


}  // namespace vc4
}  // namespace QPULib


#endif  // _QPULIB_VC4_SOURCETRANSLATE_H_
