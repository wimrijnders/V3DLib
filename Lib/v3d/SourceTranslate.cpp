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


void SourceTranslate::setupVPMWriteStmt(Seq<Instr>* seq, Stmt *s) {
	// ignore
}


/**
 * @param seq  list of generated instructions up till now
 */
void SourceTranslate::storeRequest(Seq<Instr>* seq, Expr* data, Expr* addr) {
	printf("Entered storeRequest for v3d\n");

  if (data->tag != VAR || addr->tag != VAR) {
    data = putInVar(seq, data);
    addr = putInVar(seq, addr);
  }

	// Output should be something like:
	//
	// mov(tmud, reg_sum)
  // mov(tmua, reg_dst)

	Reg srcAddr = srcReg(addr->var);
  Reg tmud;
  tmud.tag = SPECIAL;
  tmud.regId = SPECIAL_VPM_WRITE;
  seq->append(genOR(tmud, srcAddr, srcAddr));

	Reg srcData = srcReg(data->var);
  Reg tmua;
  tmua.tag = SPECIAL;
  tmua.regId = SPECIAL_DMA_ST_ADDR;
  seq->append(genOR(tmua, srcData, srcData));
}

}  // namespace v3d
}  // namespace QPULib
