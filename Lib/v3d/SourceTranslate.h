#ifndef _V3DLIB_V3D_SOURCETRANSLATE_H_
#define _V3DLIB_V3D_SOURCETRANSLATE_H_
#include "../SourceTranslate.h"

namespace V3DLib {
namespace v3d {

class SourceTranslate : public ISourceTranslate {
public:
	Instr::List deref_var_var(Var lhs, Var rhs) override;
	void varassign_deref_var(Instr::List* seq, Var &v, Expr &e) override;
	void regAlloc(CFG* cfg, Instr::List* instrs) override;
	bool stmt(Instr::List &seq, Stmt::Ptr s) override; 
};

void add_init(Instr::List &code);

}  // namespace v3d
}  // namespace V3DLib


#endif  // _V3DLIB_iV3D_SOURCETRANSLATE_H_
