#ifndef _V3DLIB_VC4_SOURCETRANSLATE_H_
#define _V3DLIB_VC4_SOURCETRANSLATE_H_
#include "../SourceTranslate.h"

namespace V3DLib {
namespace vc4 {

class SourceTranslate : public ISourceTranslate {
public:
	Instr::List deref_var_var(Var lhs, Var rhs) override;
	void varassign_deref_var(Instr::List *seq, Var &v, Expr &e) override;
	void regAlloc(CFG* cfg, Instr::List *instrs) override;
	bool stmt(Instr::List &seq, Stmt::Ptr s) override; 
};


}  // namespace vc4
}  // namespace V3DLib


#endif  // _V3DLIB_VC4_SOURCETRANSLATE_H_
