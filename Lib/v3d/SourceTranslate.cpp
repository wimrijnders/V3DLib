#include "SourceTranslate.h"
#include "Support/basics.h"
#include "Source/Translate.h"  // srcReg()
#include "Target/Liveness.h"
#include "Target/Subst.h"

namespace V3DLib {

namespace {

/**
 * @param seq  list of generated instructions up till now
 */
void storeRequest(Seq<Instr>* seq, Expr* data, Expr* addr) {
	using namespace V3DLib::Target::instr;

  if (addr->tag() != VAR || data->tag() != VAR) {
    addr = putInVar(seq, addr);
    data = putInVar(seq, data);
  }

	Reg srcAddr = srcReg(addr->var);
	Reg srcData = srcReg(data->var);

  *seq << mov(TMUD, srcData);
  seq->back().comment("Store request");
  *seq  << mov(TMUA, srcAddr);
}

}  // anon namespace


namespace v3d {

/**
 * Case: *v := rhs where v is a var and rhs is a var
 */
bool SourceTranslate::deref_var_var(Seq<Instr>* seq, Expr &lhs, ExprPtr rhs) {
	using namespace V3DLib::Target::instr;
	assert(seq != nullptr);
	assert(rhs.get() != nullptr);

	Reg srcAddr = srcReg(rhs->var);
	Reg srcData = srcReg(lhs.deref.ptr->var);

	if (rhs->var.tag() == ELEM_NUM) {
		//TODO: is ACC0 safe here?
		assert(srcAddr == ELEM_ID);
		*seq << mov(ACC0, ELEM_ID)
		     << mov(TMUD, ACC0);
	} else {
		*seq << mov(TMUD, srcAddr);
	}

	*seq << mov(TMUA, srcData)
	     << tmuwt();

	return true;
}


/**
 * Case: v := *w where w is a variable
 */
void SourceTranslate::varassign_deref_var(Seq<Instr>* seq, Var &v, Expr &e) {
	using namespace V3DLib::Target::instr;
	assert(seq != nullptr);

  Instr ldtmu_r4;
	ldtmu_r4.tag = TMU0_TO_ACC4;

	Reg src = srcReg(e.deref.ptr->var);
	*seq << mov(TMU0_S, src)

	     // TODO: Do we need NOP's here?
	     // TODO: Check if more fields need to be set
	     // TODO is r4 safe? Do we need to select an accumulator in some way?
	     << Instr::nop()
	     << Instr::nop()
	     << ldtmu_r4
	     << mov(dstReg(v), ACC4);
}


void SourceTranslate::regAlloc(CFG* cfg, Seq<Instr>* instrs) {
  int numVars = getFreshVarCount();

  // Step 0
  // Perform liveness analysis
  Liveness live(*cfg);
  live.compute(*instrs);
	assert(instrs->size() == live.size());

  // Step 2
  // For each variable, determine all variables ever live at the same time
  LiveSets liveWith(numVars);
	liveWith.init(*instrs, live);

  // Step 3
  // Allocate a register to each variable
  std::vector<Reg> alloc(numVars);
  for (int i = 0; i < numVars; i++) alloc[i].tag = NONE;

	// Allocate registers to the variables
  for (int i = 0; i < numVars; i++) {
		auto possible = liveWith.possible_registers(i, alloc);

    alloc[i].tag = REG_A;
    RegId regId = LiveSets::choose_register(possible, false);

		if (regId < 0) {
			std::string buf = "v3d regAlloc(): register allocation failed for target instruction ";
			buf << i << ": " << (*instrs)[i].mnemonic();
			error(buf, true);
		} else {
    	alloc[i].regId = regId;
		}
  }

  // Step 4
  // Apply the allocation to the code
  for (int i = 0; i < instrs->size(); i++) {
		auto &useDefSet = liveWith.useDefSet;
    Instr &instr = instrs->get(i);

    useDef(instr, &useDefSet);
    for (int j = 0; j < useDefSet.def.size(); j++) {
      RegId r = useDefSet.def[j];
      renameDest(&instr, REG_A, r, TMP_A, alloc[r].regId);
    }
    for (int j = 0; j < useDefSet.use.size(); j++) {
      RegId r = useDefSet.use[j];
      renameUses(&instr, REG_A, r, TMP_A, alloc[r].regId);
    }
    substRegTag(&instr, TMP_A, REG_A);
  }
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
	using namespace V3DLib::Target::instr;

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
	ret << mov(rf(RSV_QPU_ID), 0)           // not needed, already init'd to 0. Left here to counter future brainfarts
	    << sub(ACC0, rf(RSV_NUM_QPUS), 8).pushz()
	    << branch(endifLabel).allzc()       // nop()'s added downstream
			<< mov(ACC0, QPU_ID)
			<< shr(ACC0, ACC0, 2)
	    << band(rf(RSV_QPU_ID), ACC0, 15)
			<< label(endifLabel)
	;

	// offset = 4 * (thread_num + 16 * qpu_num);
	ret << shl(ACC1, rf(RSV_QPU_ID), 4) // Avoid ACC0 here, it's used for getting QPU_ID and ELEM_ID (next stmt)
			<< mov(ACC0, ELEM_ID)
			<< add(ACC1, ACC1, ACC0)
	    << shl(ACC0, ACC1, 2)           // Post: offset now in ACC0
	    << add_uniform_pointer_offset(code);

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
}  // namespace V3DLib
