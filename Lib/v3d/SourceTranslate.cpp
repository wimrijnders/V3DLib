#include "SourceTranslate.h"
#include "Support/debug.h"
#include "Source/Translate.h"  // srcReg()
#include "Target/LoadStore.h"  // move_from_r4()
#include "Target/Liveness.h"
#include "Target/Subst.h"

namespace QPULib {

namespace {

/**
 * @param seq  list of generated instructions up till now
 */
void storeRequest(Seq<Instr>* seq, Expr* data, Expr* addr) {
	using namespace QPULib::Target::instr;

  if (addr->tag != VAR || data->tag != VAR) {
    addr = putInVar(seq, addr);
    data = putInVar(seq, data);
  }

	Reg srcAddr = srcReg(addr->var);
  Reg tmud(SPECIAL, SPECIAL_VPM_WRITE);
  *seq << mov(tmud, srcAddr);

	Reg srcData = srcReg(data->var);
  Reg tmua(SPECIAL, SPECIAL_DMA_ST_ADDR);
  *seq << mov(tmua, srcData);
}

}  // anon namespace

namespace v3d {

bool SourceTranslate::deref_var_var(Seq<Instr>* seq, Expr &lhs, Expr *rhs) {
	assert(seq != nullptr);
	assert(rhs != nullptr);
	using namespace QPULib::Target::instr;
	Seq<Instr> &ret = *seq;

	Reg dst(SPECIAL, SPECIAL_VPM_WRITE);

	if (rhs->var.tag == ELEM_NUM) {
		debug("TODO: is ACC0 safe here?");
		assert(srcReg(rhs->var) == ELEM_ID);
		ret << mov(ACC0, ELEM_ID)
		    << mov(dst, ACC0);
	} else {
		ret << mov(dst, srcReg(rhs->var));
	}

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

	dst = dstReg(v);
	ret << move_from_r4(dst);
}


void SourceTranslate::regAlloc(CFG* cfg, Seq<Instr>* instrs) {
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


Instr cond_branch(Label label) {
	Instr instr;
	instr.tag          = BRL;
	instr.BRL.cond.tag = COND_ALWAYS;    // Will be set with another call
	instr.BRL.label    = label;

	return instr;
}


Instr label(Label in_label) {
	Instr instr;
	instr.tag = LAB;
	instr.label(in_label);

	return instr;
}


/**
 * Add extra initialization code after uniform loads
 */
void SourceTranslate::add_init(Seq<Instr> &code) {
	using namespace QPULib::Target::instr;

	int insert_index = get_init_begin_marker(code);
	Seq<Instr> ret;
	Label endifLabel = freshLabel();

	// Determine the qpu index for 'current' QPU
	// This is derived from the thread index. 
	//
	// Broadly:
	//
	// If (numQPUs() == 8)  // Alternative is 1, then qpu num initalized to 0 is ok
	// 	me() = (thread_index() >> 2) & 0b1111;
	// End
	//
	// This works because the thread indexes are consecutive for multiple reserved
	// threads. It's probably also the reason why you can select only 1 or 8 (max)
	// threads, otherwise there would be gaps in the qpu id.
	//
	ret << mov(rf(RSV_QPU_ID), 0);
	ret << sub(ACC0, rf(RSV_NUM_QPUS), 8).setFlags()
	    << cond_branch(endifLabel).allzc()  // nop()'s added downstream
			<< mov(ACC0, QPU_ID)
			<< shr(ACC0, ACC0, 2)
	    << band(rf(RSV_QPU_ID), ACC0, 15)
			<< label(endifLabel);

	// offset = 4 * (thread_num + 16 * qpu_num);
	ret << shl(ACC1, rf(RSV_QPU_ID), 4) // Avoid ACC0 here, it's used for getting QPU_ID and ELEM_ID (next stmt)
			<< mov(ACC0, ELEM_ID)
			<< add(ACC1, ACC1, ACC0)
	    << shl(ACC0, ACC1, 2);          // Post: offset in ACC0

	ret << add_uniform_pointer_offset(code);

	code.insert(insert_index + 1, ret);  // Insert init code after the INIT_BEGIN marker
}


/**
 * @return true if statement handled, false otherwise
 */
bool SourceTranslate::stmt(Seq<Instr>* seq, Stmt* s) {

  // ---------------------------------------------
  // Case: store(e0, e1) where e1 and e2 are exprs
  // ---------------------------------------------
  if (s->tag == STORE_REQUEST) {
		storeRequest(seq, s->storeReq.data, s->storeReq.addr);
    return true;
  }

	return false;
}

}  // namespace v3d
}  // namespace QPULib
