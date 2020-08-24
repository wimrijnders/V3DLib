#include "SourceTranslate.h"
#include "../Support/debug.h"
#include "../Source/Translate.h"  // srcReg()

namespace QPULib {
namespace v3d {

bool SourceTranslate::deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr *rhs) {
	assert(seq != nullptr);
	assert(rhs != nullptr);

	// TODO: offset by index

	Reg dst;
	dst.tag = SPECIAL;
	dst.regId = SPECIAL_VPM_WRITE;
	seq->append(genMove(dst, srcReg(rhs->var)));

	dst.regId = SPECIAL_DMA_ST_ADDR;
	seq->append(genMove(dst, srcReg(lhs.deref.ptr->var)));

	return true;
}

}  // namespace v3d
}  // namespace QPULib
