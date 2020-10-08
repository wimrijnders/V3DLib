#include <assert.h>
#include "Source/Syntax.h"
#include "Target/Syntax.h"

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


Instr genInstr(ALUOp op, AssignCond cond, Reg dst, Reg srcA, Reg srcB) {
  Instr instr(ALU);
  instr.ALU.cond      = cond;
  instr.ALU.dest      = dst;
  instr.ALU.srcA.tag  = REG;
  instr.ALU.srcA.reg  = srcA;
  instr.ALU.op        = op;
  instr.ALU.srcB.tag  = REG;
  instr.ALU.srcB.reg  = srcB;

  return instr;
}


namespace {

Instr genInstr(ALUOp op, Reg dst, Reg srcA, Reg srcB) {
	return genInstr(op, always, dst, srcA, srcB);
}


Instr genInstr(ALUOp op, Reg dst, Reg srcA, int n) {
  Instr instr(ALU);
  instr.ALU.dest              = dst;
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg          = srcA;
  instr.ALU.op                = op;
  instr.ALU.srcB.tag          = IMM;
  instr.ALU.srcB.smallImm.tag = SMALL_IMM;
  instr.ALU.srcB.smallImm.val = n;

  return instr;
}


Instr genInstr(ALUOp op, Reg dst, int n, int m) {
  Instr instr(ALU);
  instr.ALU.dest              = dst;
  instr.ALU.srcA.tag          = IMM;
  instr.ALU.srcA.smallImm.tag = SMALL_IMM;
  instr.ALU.srcA.smallImm.val = n;
  instr.ALU.op                = op;
  instr.ALU.srcB.tag          = IMM;
  instr.ALU.srcB.smallImm.tag = SMALL_IMM;
  instr.ALU.srcB.smallImm.val = m;

  return instr;
}


int globalLabelId = 0;  // Used for fresh label generation

}  // anon namespace


/**
 * Function to negate a condition flag
 */
Flag negFlag(Flag flag) {
  switch(flag) {
    case ZS: return ZC;
    case ZC: return ZS;
    case NS: return NC;
    case NC: return NS;
  }

  // Not reachable
  assert(false);
	return ZS;
}


namespace {

const char *pretty(Flag flag) {
  switch (flag) {
    case ZS: return "ZS";
    case ZC: return "ZC";
    case NS: return "NS";
    case NC: return "NC";
		default: assert(false); return "";
  }
}

}  // anon namespace


///////////////////////////////////////////////////////////////////////////////
// Class BranchCond
///////////////////////////////////////////////////////////////////////////////

std::string BranchCond::to_string() const {
	std::string ret;

  switch (tag) {
    case COND_ALL:
      ret << "all(" << pretty(flag) << ")";
      break;
    case COND_ANY:
      ret << "any(" << pretty(flag) << ")";
      break;
    case COND_ALWAYS:
      ret = "always";
      break;
    case COND_NEVER:
      ret = "never";
      break;
  }

	return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Class AssignCond
///////////////////////////////////////////////////////////////////////////////

namespace {

std::string pretty(AssignCond cond) {
	using Tag = AssignCond::Tag;

  switch (cond.tag) {
    case Tag::ALWAYS: return "always";
    case Tag::NEVER:  return "never";
    case Tag::FLAG:   return pretty(cond.flag);
		default: assert(false); return "";
  }
}

}  // anon namespace


AssignCond always(AssignCond::Tag::ALWAYS);  // Is a global to reduce eyestrain in gdb
AssignCond never(AssignCond::Tag::NEVER);    // idem


AssignCond AssignCond::negate() const {
	AssignCond ret = *this;

  switch (tag) {
    case NEVER:  ret.tag = ALWAYS; break;
    case ALWAYS: ret.tag = NEVER;  break;
    case FLAG:   ret.flag = negFlag(flag); break;
		default:
			assert(false);
			break;
  }

	return ret;
}


std::string AssignCond::to_string() const {
	auto ALWAYS = AssignCond::Tag::ALWAYS;

	if (tag == ALWAYS) {
		return "";
	} else {
		std::string ret;
		ret << "where " << pretty(*this) << ": ";
		return ret;
	}
}


///////////////////////////////////////////////////////////////////////////////
// Class BranchTarget
///////////////////////////////////////////////////////////////////////////////

std::string BranchTarget::to_string() const {
	std::string ret;

	if (relative) ret << "PC+1+";
	if (useRegOffset) ret << "A" << regOffset << "+";
	ret << immOffset;

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// End Class BranchTarget
///////////////////////////////////////////////////////////////////////////////


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


// ======================
// Handy syntax functions
// ======================

// Is last instruction in a basic block?
bool isLast(Instr instr) {
  return instr.tag == BRL || instr.tag == BR || instr.tag == END;
}

// =========================
// Fresh variable generation
// =========================

// Obtain a fresh variable
Reg freshReg()
{
  Var v = freshVar();
  Reg r;
  r.tag = REG_A;
  r.regId = v.id;
  return r;
}

// Obtain a fresh label
Label freshLabel()
{
  return globalLabelId++;
}

// Number of fresh labels
int getFreshLabelCount()
{
  return globalLabelId;
}

// Reset fresh label generator
void resetFreshLabelGen()
{
  globalLabelId = 0;
}

// Reset fresh label generator to specified value
void resetFreshLabelGen(int val)
{
  globalLabelId = val;
}


namespace Target {
namespace instr {

Reg const None(NONE, 0);
Reg const ACC0(ACC, 0);
Reg const ACC1(ACC, 1);
Reg const ACC4(ACC, 4);
Reg const QPU_ID(     SPECIAL, SPECIAL_QPU_NUM);
Reg const ELEM_ID(    SPECIAL, SPECIAL_ELEM_NUM);
Reg const TMU0_S(     SPECIAL, SPECIAL_TMU0_S);
Reg const VPM_WRITE(  SPECIAL, SPECIAL_VPM_WRITE);
Reg const VPM_READ(   SPECIAL, SPECIAL_VPM_READ);
Reg const WR_SETUP(   SPECIAL, SPECIAL_WR_SETUP);
Reg const RD_SETUP(   SPECIAL, SPECIAL_RD_SETUP);
Reg const DMA_LD_WAIT(SPECIAL, SPECIAL_DMA_LD_WAIT);
Reg const DMA_ST_WAIT(SPECIAL, SPECIAL_DMA_ST_WAIT);
Reg const DMA_LD_ADDR(SPECIAL, SPECIAL_DMA_LD_ADDR);
Reg const DMA_ST_ADDR(SPECIAL, SPECIAL_DMA_ST_ADDR);

// Synonyms for v3d
Reg const TMUD(SPECIAL, SPECIAL_VPM_WRITE);
Reg const TMUA(SPECIAL, SPECIAL_DMA_ST_ADDR);

Reg rf(uint8_t index) {
	return Reg(REG_A, index);
}


Instr mov(Reg dst, int n) {
	return genInstr(A_BOR, dst, n, n);
}


Instr mov(Reg dst, Reg src, AssignCond cond) {
	return genInstr(A_BOR, cond, dst, src, src);
}


Instr mov(Reg dst, Reg src) {
	return bor(dst, src, src);
}



/**
 * Generate bitwise-or instruction.
 */
Instr bor(Reg dst, Reg srcA, Reg srcB) {
	return genInstr(A_BOR, dst, srcA, srcB);
}


/**
 * Generate left-shift instruction.
 */
Instr shl(Reg dst, Reg srcA, int val) {
  assert(val >= 0 && val <= 15);
	return genInstr(A_SHL, dst, srcA, val);
}


Instr shr(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
	return genInstr(A_SHR, dst, srcA, n);
}


/**
 * Generate addition instruction.
 */
Instr add(Reg dst, Reg srcA, Reg srcB) {
	return genInstr(A_ADD, dst, srcA, srcB);
}


Instr add(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
	return genInstr(A_ADD, dst, srcA, n);
}


Instr sub(Reg dst, Reg srcA, int n) {
  assert(n >= 0 && n <= 15);
	return genInstr(A_SUB, dst, srcA, n);
}


Instr band(Reg dst, Reg srcA, int n) {
	return genInstr(A_BAND, dst, srcA, n);
}


/**
 * Generate load-immediate instruction.
 */
Instr li(AssignCond cond, Reg dst, int i) {
  Instr instr(LI);
  instr.LI.cond       = cond;
  instr.LI.dest       = dst;
  instr.LI.imm.tag    = IMM_INT32;
  instr.LI.imm.intVal = i;
 
  return instr;
}


/**
 * Generate load-immediate instruction.
 */
Instr li(Reg dst, int i) {
	return li(always, dst, i);
}


/**
 * Create an unconditional branch.
 *
 * Conditions can still be specified with helper methods (e.g. see `allzc()`)
 */
Instr branch(Label label) {
	Instr instr;
	instr.tag          = BRL;
	instr.BRL.cond.tag = COND_ALWAYS;    // Can still be changed
	instr.BRL.label    = label;

	return instr;
}

/**
 * Create label instruction.
 *
 * This is a meta-instruction for Target source.
 */
Instr label(Label in_label) {
	Instr instr;
	instr.tag = LAB;
	instr.label(in_label);

	return instr;
}



/**
 * Create a conditional branch.
 */
Instr branch(BranchCond cond, Label label) {
	Instr instr;
	instr.tag       = BRL;
	instr.BRL.cond  = cond; 
	instr.BRL.label = label;

	return instr;
}


/**
 * v3d only
 */
Instr tmuwt() {
	Instr instr;
	instr.tag = TMUWT;
	return instr;
}

}  // namespace instr
}  // namespace Target

}  // namespace QPULib
