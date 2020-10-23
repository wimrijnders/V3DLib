#include "Syntax.h"         // Location of definition struct Instr
#include "Target/Pretty.h"  // pretty_instr_tag()
#include "Support/basics.h" // fatal()

namespace QPULib {

/**
 * Initialize the fields per selected instruction tag.
 *
 * Done like this, because union members can't have non-trivial constructors.
 */
Instr::Instr(InstrTag in_tag) {
	switch (in_tag) {
	case InstrTag::ALU:
  	tag          = InstrTag::ALU;
	  ALU.setFlags = false;
	  ALU.setCond  = SetCond::NO_COND;
	  ALU.cond     = always;
		break;
	case InstrTag::LI:
    tag          = InstrTag::LI;
    LI.setFlags  = false;
	  LI.setCond   = SetCond::NO_COND;
	  LI.cond      = always;
		break;
	case InstrTag::RECV:
	case InstrTag::PRI:
    tag          = in_tag;
		break;
	default:
		assert(false);
		break;
	}
}


/**
 * Determines if the mul-ALU needs to be used
 *
 * TODO: Examine if this is still true for v3d
 */
bool Instr::isMul() const {
	auto op = ALU.op;

  bool ret =
		op == M_FMUL   || op == M_MUL24 || op == M_V8MUL  ||
    op == M_V8MIN  || op == M_V8MAX || op == M_V8ADDS ||
    op == M_V8SUBS || op == M_ROTATE;

	return ret;
}


Instr Instr::nop() {
	Instr instr;
	instr.tag = NO_OP;
	return instr;
}


/**
 * Determine if instruction is a conditional assignment
 */
bool Instr::isCondAssign() const {
  if (tag == InstrTag::LI && !LI.cond.is_always())
    return true;

  if (tag == InstrTag::ALU && !ALU.cond.is_always())
    return true;

  return false;
}


/**
 * Determine if this is the last instruction in a basic block
 *
 * TODO Unused, do we need this?
 */
bool Instr::isLast() const {
  return tag == QPULib::BRL || tag == QPULib::BR || tag == QPULib::END;
}


void Instr::setFlags() {
	if (tag == InstrTag::LI) {
		LI.setFlags = true;
	} else if (tag == InstrTag::ALU) {
		ALU.setFlags = true;
	} else {
		assert(false);
	}
}


Instr &Instr::pushz() {
	setFlags();

	if (tag == InstrTag::LI) {
		LI.setCond = SetCond::Z;
	} else if (tag == InstrTag::ALU) {
		ALU.setCond = SetCond::Z;
	} else {
		assert(false);
	}

	return *this;
}


/**
 * Convert branch label to branch target
 * 
 * @param offset  offset to the label from current instruction
 */
void Instr::label_to_target(int offset) {
	assert(tag == InstrTag::BRL);

	// Convert branch label (BRL) instruction to branch instruction with offset (BR)
	// Following assumes that BranchCond field 'cond' survives the transition to another union member

	BranchTarget t;
	t.relative       = true;
	t.useRegOffset   = false;
	t.immOffset      = offset - 4;  // Compensate for the 4-op delay for executing a branch

	tag        = InstrTag::BR;
	BR.target  = t;
}



bool Instr::isUniformLoad() const {
	if (tag == TMU0_TO_ACC4 || tag == InstrTag::LI) {
		return false;
	}

	if (ALU.srcA.tag != REG || ALU.srcB.tag != REG) {
		return false;  // Both operands must be regs
	}

	Reg aReg  = ALU.srcA.reg;
	Reg bReg  = ALU.srcB.reg;

	if (aReg.tag == SPECIAL && aReg.regId == SPECIAL_UNIFORM) {
		assert(aReg == bReg);  // Apparently, this holds (NOT TRUE)
		return true;
	} else {
		assert(!(bReg.tag == SPECIAL && bReg.regId == SPECIAL_UNIFORM));  // not expecting this to happen
		return false;
	}
}


bool Instr::isTMUAWrite() const {
 	if (tag != InstrTag::ALU) {
		return false;
	}
	Reg reg = ALU.dest;

	bool ret = (reg.regId == SPECIAL_DMA_ST_ADDR)
	        || (reg.regId == SPECIAL_TMU0_S);

	if (ret) {
		// It's a simple move (BOR) instruction, src registers should be the same
		auto reg_a = ALU.srcA;
		auto reg_b = ALU.srcB;
		if (reg_a != reg_b) {
			breakpoint
		}
		assert(reg_a == reg_b);

		// In current logic, src should always be read from register file;
		// enforce this.
		assert(reg_a.tag == REG
		    && (reg_a.reg.tag == REG_A || reg_a.reg.tag == REG_B));
	}

	return ret;
}


/**
 * Determine if this instruction has all fields set to zero.
 *
 * This is an illegal instruction, but has popped up.
 */
bool Instr::isZero() const {
	return tag == InstrTag::LI
		  && LI.setFlags   == false
		  && LI.setCond    == NO_COND
		  && LI.cond.tag   == AssignCond::NEVER
		  && LI.cond.flag  == ZS
		  && LI.dest.tag   == REG_A
		  && LI.dest.regId == 0
		  && LI.dest.isUniformPtr == false 
		  && LI.imm.tag    == IMM_INT32
		  && LI.imm.intVal == 0
	;
}


/**
 * Check if given tag is for the specified platform
 */
void check_instruction_tag_for_platform(InstrTag tag, bool for_vc4) {
	char const *platform = nullptr;

	if (for_vc4) {
		if (tag >= V3D_ONLY && tag < END_V3D_ONLY) {
			platform = "vc4";
		} 
	} else {  // v3d
		if (tag >= VC4_ONLY && tag < END_VC4_ONLY) {
			platform = "v3d";
		}
	}

	if (platform != nullptr) {
		std::string msg = "Instruction tag ";
		msg << pretty_instr_tag(tag) << "(" + std::to_string(tag) + ")" << " can not be used on " << platform;
		fatal(msg);
	}
}


/**
 * Debug function - check for presence of zero-instructions in instruction sequence
 *
 */
void check_zeroes(Seq<Instr> const &instrs) {
	bool success = true;

	for (int i = 0; i < instrs.size(); ++i ) {
		if (instrs[i].isZero()) {
			std::string msg = "Zero instruction encountered at position ";
			// Grumbl not working:  msg << i;
			msg += std::to_string(i);
			warning(msg.c_str());

			success = false;
		}
	}

	if (!success) {
		error("zeroes encountered in instruction sequence", true);
	}
}


/**
 * Returns a string representation of an instruction.
 */
std::string Instr::mnemonic(bool with_comments) const {
	return pretty(*this, with_comments);
}

}  // namespace QPULib
