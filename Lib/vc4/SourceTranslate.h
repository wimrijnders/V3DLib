#ifndef _V3DLIB_VC4_SOURCETRANSLATE_H_
#define _V3DLIB_VC4_SOURCETRANSLATE_H_
#include "../SourceTranslate.h"

namespace V3DLib {
namespace vc4 {

class SourceTranslate : public ISourceTranslate {
public:
	bool deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr::Ptr rhs) override;
	void varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) override;
	void regAlloc(CFG* cfg, Seq<Instr>* instrs) override;
	bool stmt(Seq<Instr> &seq, Stmt* s) override; 
};


}  // namespace vc4
}  // namespace V3DLib


#endif  // _V3DLIB_VC4_SOURCETRANSLATE_H_
