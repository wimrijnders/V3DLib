#include "SourceTranslate.h"
#include "Support/debug.h"
#include "Source/Translate.h"  // srcReg()
#include "Target/LoadStore.h"  // move_from_r4()
#include "Target/Liveness.h"
#include "Target/Subst.h"

namespace QPULib {
namespace v3d {

bool SourceTranslate::deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr *rhs) {
	assert(seq != nullptr);
	assert(rhs != nullptr);
	using namespace QPULib::Target::instr;
	Seq<Instr> &ret = *seq;

	Reg dst(SPECIAL, SPECIAL_VPM_WRITE);
	ret << mov(dst, srcReg(rhs->var));

	dst.regId = SPECIAL_DMA_ST_ADDR;
	ret << mov(dst, srcReg(lhs.deref.ptr->var));

	return true;
}


/**
 * See comment and preamble code in caller: Target/Translate.cpp, line 52
 *
 * Basically the same as `deref_var_var()` above.
 */
void SourceTranslate::varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) {
	// TODO: Check if offset by index needed
	assert(seq != nullptr);
	using namespace QPULib::Target::instr;
	Seq<Instr> ret = *seq;

	Reg src = srcReg(e.deref.ptr->var);

	Reg dst(SPECIAL,SPECIAL_TMU0_S);
	ret << mov(dst, src);

	// TODO: Do we need NOP's here?
	// TODO: Check if more fields need to be set
	// TODO is r4 safe? Do we need to select an accumulator in some way?
  Instr instr;
	instr.tag = TMU0_TO_ACC4;
	ret << instr;

	dst = srcReg(v);
	ret << move_from_r4(dst);
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

	// Output should be:
	//
	// mov(tmud, data)
  // mov(tmua, addr)

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


void SourceTranslate::regAlloc(CFG* cfg, Seq<Instr>* instrs) {
	//breakpoint
  int n = getFreshVarCount();

  // Step 0
  // Perform liveness analysis
  Liveness live(*cfg);
  live.compute(instrs);

  // Step 2
  // For each variable, determine all variables ever live at same time
  LiveSets liveWith(n);
	liveWith.init(instrs, live);

  // Step 3
  // Allocate a register to each variable
  std::vector<Reg> alloc(n);
  for (int i = 0; i < n; i++) alloc[i].tag = NONE;

	// Allocate registers to the variables
  for (int i = 0; i < n; i++) {
		auto possible = liveWith.possible_registers(i, alloc);

    alloc[i].tag = REG_A;
    alloc[i].regId = LiveSets::choose_register(possible);
  }

  // Step 4
  // Apply the allocation to the code
  for (int i = 0; i < instrs->numElems; i++) {
		auto &useDefSet = liveWith.useDefSet;
    Instr* instr = &instrs->elems[i];

    useDef(*instr, &useDefSet);
    for (int j = 0; j < useDefSet.def.numElems; j++) {
      RegId r = useDefSet.def.elems[j];
      renameDest(instr, REG_A, r, TMP_A, alloc[r].regId);
    }
    for (int j = 0; j < useDefSet.use.numElems; j++) {
      RegId r = useDefSet.use.elems[j];
      renameUses(instr, REG_A, r, TMP_A, alloc[r].regId);
    }
    substRegTag(instr, TMP_A, REG_A);
  }
}


/**
 * Add extra initialization code after uniform loads
 */
void SourceTranslate::add_init(Seq<Instr> &code) {
	using namespace QPULib::Target::instr;

	// Find first instruction after uniform loads
	int index = 0;
	for (; index < code.size(); ++index) {
		if (!code[index].isUniformLoad()) break; 
	}
	assertq(index >= 2, "Expecting at least two uniform loads.");

breakpoint
	code.insert(index, mov(rf(0), QPU_ID));  // Regfile index 0 is reserved location for qpu id
 }

}  // namespace v3d
}  // namespace QPULib
