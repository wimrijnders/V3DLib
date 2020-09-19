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
	//printf("Entered storeRequest for v3d\n");

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

	// Find the init begin marker
	int insert_index = 0;
	for (; insert_index < code.size(); ++insert_index) {
		if (code[insert_index].tag == INIT_BEGIN) break; 
	}
	assertq(insert_index < code.size(), "Expecting INIT_BEGIN marker.");
	assertq(insert_index >= 2, "Expecting at least two uniform loads.");

	Seq<Instr> ret;

/*
16: A0 <-{sf} sub(A0, 3)
17: if all(ZC) goto PC+1+3
18: NOP
19: NOP
20: NOP
21: ACC1 <- 7
22: S[VPM_WRITE] <- or(ACC1, ACC1)
23: S[DMA_ST_ADDR] <- or(A1, A1)
*/
	//if (QPU_NUM == 8) {
	//ret << sub(ACC0, rf(RSV_NUM_QPUS), 8).setFlags()

	ret << mov(ACC1, QPU_ID)
	    << shr(ACC1, ACC1, 2)
	    << band(rf(RSV_QPU_ID), ACC1, 0b1111)
	    << nop();                       // Kludge: prevent the peephole optimization `introduceAccum()` from kicking in

	// offset = 4 * (thread_num + 16 * qpu_num)";
	ret << shl(ACC1, rf(RSV_QPU_ID), 4) // Avoid ACC0 here, it's used for getting QPU_ID and ELEM_ID
			<< add(ACC1, ACC1, ELEM_ID)     // ELEM_ID uses ACC0
	    << shl(ACC0, ACC1, 2);          // offset cwnow in ACC0
/*
	ret << mov(ACC1, ELEM_ID)     // ELEM_ID uses ACC0
	    << shl(ACC0, ACC1, 2);          // offset now in ACC0
*/

	// add the offset to all the uniform pointers
	for (int index = 0; index < code.size(); ++index) {
		auto &instr = code[index];

		if (!instr.isUniformLoad()) {  // Assumption: uniform loads always at top
			break;
		}

		if (instr.ALU.srcA.tag == REG && instr.ALU.srcA.reg.isUniformPtr) {
			ret << add(rf((uint8_t) index), rf((uint8_t) index), ACC0);
		}
	}


	code.insert(insert_index + 1, ret);  // Insert init code after the INIT_BEGIN marker
 }

}  // namespace v3d
}  // namespace QPULib
