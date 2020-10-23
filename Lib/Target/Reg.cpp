#include "Reg.h"
#include "Syntax.h"
#include "Support/basics.h"

namespace QPULib {

/**
 * Obtain a register for a fresh variable
 */
Reg freshReg() {
	Var v = freshVar();
	Reg r;

	r.tag = REG_A;
	r.regId = v.id;
	return r;
}


/**
 * Translate variable to source register.
 */
Reg srcReg(Var v) {
  Reg r;
	r.isUniformPtr = false;

  switch (v.tag) {
    case UNIFORM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_UNIFORM;
			r.isUniformPtr = v.isUniformPtr;
			break;
    case QPU_NUM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_QPU_NUM;
			break;
    case ELEM_NUM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_ELEM_NUM;
			break;
    case VPM_READ:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_VPM_READ;
			break;
    case STANDARD:
      r.tag   = REG_A;
      r.regId = v.id;
			break;
    case VPM_WRITE:
    case TMU0_ADDR:
      printf("QPULib: Reading from write-only special register is forbidden\n");
      assert(false);
			break;
    case DUMMY:
      r.tag   = NONE;
      r.regId = v.id;
			break;
		default:
			assert(false);
			break;
  }

	return r;
}


/**
 * Translate variable to target register.
 */
Reg dstReg(Var v) {
	using namespace QPULib::Target::instr;

  switch (v.tag) {
    case UNIFORM:
    case QPU_NUM:
    case ELEM_NUM:
    case VarTag::VPM_READ:
      fatal("QPULib: writing to read-only special register is forbidden");
			return Reg();  // Return anything

    case STANDARD:
			return Reg(REG_A, v.id);
    case VarTag::VPM_WRITE:
			return Target::instr::VPM_WRITE;
    case TMU0_ADDR:
			return TMU0_S;

		default:
			fatal("Unhandled case in dstReg()");
			return Reg();  // Return anything
  }
}

}  // namespace QPULib
