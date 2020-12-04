#ifndef _V3DLIB_V3D_SOURCETRANSLATE_H_
#define _V3DLIB_V3D_SOURCETRANSLATE_H_
#include "../SourceTranslate.h"

namespace V3DLib {
namespace v3d {

class SourceTranslate : public ISourceTranslate {
public:
	Seq<Instr> deref_var_var(Var lhs, Var rhs) override;
	void varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) override;
	void regAlloc(CFG* cfg, Seq<Instr>* instrs) override;
	bool stmt(Seq<Instr> &seq, Stmt* s) override; 
};

void add_init(Seq<Instr> &code);

}  // namespace v3d
}  // namespace V3DLib


#endif  // _V3DLIB_iV3D_SOURCETRANSLATE_H_
