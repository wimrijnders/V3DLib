/**
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * Setting of branch conditions
 *
 * * qpu_instr.h, line 74, enum v3d_qpu_uf:
 *
 *   How I interpret this:
 *   - AND: if all bits set
 *   - NOR: if no bits set
 *   - N  : field not set
 *   - Z  : Field zero
 *   - N  : field negative set
 *   - C  : Field negative cleared
 *
 *   What the bits are is not clear at this point.
 *   These assumptions are probably wrong, but I need a starting point.
 *
 *   TODO: make tests to verify these assumptions (how? No clue right now)
 *
 *   So:
 *   - vc4 `if all(nc)...` -> ANDC
 *     vc4 `nc` - negative clear, ie. >= 0
 *     vc4 `ns` - negative set,   ie.  < 0
 * 
 */
#include "KernelDriver.h"
#include <memory>
#include <iostream>
#include "../Support/basics.h"
#include "Target/SmallLiteral.h"  // decodeSmallLit()
#include "Target/RemoveLabels.h"
#include "Invoke.h"
#include "instr/Snippets.h"

#ifdef QPU_MODE

using std::cout;
using std::endl;

namespace QPULib {
namespace v3d {
using namespace QPULib::v3d::instr;

using Instructions = std::vector<instr::Instr>;

namespace {

uint8_t const NOP_ADDR    = 39;
uint8_t const REGB_OFFSET = 32;

uint8_t local_numQPUs = 0;
std::vector<std::string> local_errors;


uint8_t to_waddr(Reg const &reg) {
	assert(reg.tag == REG_A || reg.tag == REG_B);

	// There is no reg A and B in v3d
	// To distinguish between the register allocations,
	// an offset for B is used for now
	// TODO: clean this up
	uint8_t reg_offset = 0;

	if (reg.tag == REG_B) {
		reg_offset = REGB_OFFSET;
	}

	return (uint8_t) (reg_offset + reg.regId);
}


/**
 * Translate imm index value from vc4 to v3d
 */
SmallImm encodeSmallImm(RegOrImm const &src_reg) {
	assert(src_reg.tag == IMM);

	Word w = decodeSmallLit(src_reg.smallImm.val);
	SmallImm ret(w.intVal);

	return ret;
}


/**
 *
 */
std::unique_ptr<Location> encodeSrcReg(Reg reg) {
	bool is_none = false;
	std::unique_ptr<Location> ret;

  switch (reg.tag) {
    case REG_A:
    case REG_B:  // same as encodeDstReg()
      assert(reg.regId >= 0 && reg.regId < 32);
			ret.reset(new RFAddress(to_waddr(reg)));
			break;
    case ACC:
      assert(reg.regId >= 0 && reg.regId <= 4); // !!! Apparently, r5 not allowed here
			switch(reg.regId) {
				case 0: ret.reset(new Register(r0)); break;
				case 1: ret.reset(new Register(r1)); break;
				case 2: ret.reset(new Register(r2)); break;
				case 3: ret.reset(new Register(r3)); break;
				case 4: ret.reset(new Register(r4)); break;
			}
			break;
    case SPECIAL:
      switch (reg.regId) {
        case SPECIAL_QPU_NUM:
        case SPECIAL_ELEM_NUM:
        case SPECIAL_UNIFORM:
					breakpoint
					assertq(false, "encodeSrcReg(): Not expecting this SPECIAL regId, should be handled before call()");
				break;

				// Not handled (yet)
        case SPECIAL_VPM_READ:
        case SPECIAL_DMA_LD_WAIT:
        case SPECIAL_DMA_ST_WAIT:
					breakpoint
				break;
      }
			break;
    case NONE:
			is_none = true;
			breakpoint
			break;
  }

	if (ret.get() == nullptr && !is_none) {
		breakpoint
	  assertq(false, "QPULib: missing case in encodeSrcReg()");
	}

	return ret;
}


std::unique_ptr<Location> encodeDestReg(QPULib::Instr const &src_instr) {
	assert(!src_instr.isUniformLoad());

	bool is_none = false;
	std::unique_ptr<Location> ret;

	Reg reg;
  if (src_instr.tag == ALU) {
		reg = src_instr.ALU.dest;
	} else {
  	assert(src_instr.tag == LI);
		reg = src_instr.LI.dest;
	}

  switch (reg.tag) {
    case REG_A:
    case REG_B:
      assert(reg.regId >= 0 && reg.regId < 32);
			ret.reset(new RFAddress(to_waddr(reg)));
			break;
    case ACC:
      assert(reg.regId >= 0 && reg.regId <= 5);
			switch(reg.regId) {
				case 0: ret.reset(new Register(r0)); break;
				case 1: ret.reset(new Register(r1)); break;
				case 2: ret.reset(new Register(r2)); break;
				case 3: ret.reset(new Register(r3)); break;
				case 4: ret.reset(new Register(r4)); break;
				case 5: ret.reset(new Register(r5)); break;
			}
			break;
    case SPECIAL:
      switch (reg.regId) {
				// These values should never be generated for v3d
        case SPECIAL_RD_SETUP:            // value 6
        case SPECIAL_WR_SETUP:            // value 7
        case SPECIAL_HOST_INT:            // value 11
					breakpoint
					assert(false);                  // Do not want this
					break;

        case SPECIAL_DMA_LD_ADDR:         // value 9
					throw Exception("The source code uses DMA instructions. These are not supported for v3d.");
					break;

				// These values *are* generated and handled
				// Note that they get translated to the v3d registers, though
        case SPECIAL_VPM_WRITE:           // Write TMU, to set data to write
					ret.reset(new Register(tmud));
					break;
        case SPECIAL_DMA_ST_ADDR:         // Write TMU, to set memory address to write to
					ret.reset(new Register(tmua));
					break;
        case SPECIAL_TMU0_S:              // Read TMU
					ret.reset(new Register(tmua));
					break;

        default:
					breakpoint
					assert(false);  // Not expecting this
					break;
      }
			break;
    case NONE:
			// As far as I can tell, there is no such thing as a NONE register on v3d;
			// it may be one of the bits in `struct v3d_qpu_sig`.
			//
			// The first time I encountered this was in (QPULib target code):
			//       _ <-{sf} or(B6, B6)
			//
			// The idea seems to be to set the CNZ flags depending on the value of a given rf-register.
			// So, for the time being, we will set a condition (how? Don't know for sure yet) if
			// srcA and srcB are the same in this respect, and set target same as both src's.
			is_none = true;
			assert(src_instr.ALU.setFlags);
  		assert(src_instr.tag == ALU);

			// srcA and srcB are the same rf-register
			if ((src_instr.ALU.srcA.tag == REG && src_instr.ALU.srcB.tag == REG)
			&& ((src_instr.ALU.srcA.reg.tag == REG_A && src_instr.ALU.srcA.reg.tag == src_instr.ALU.srcB.reg.tag)
			|| (src_instr.ALU.srcA.reg.tag == REG_B && src_instr.ALU.srcA.reg.tag == src_instr.ALU.srcB.reg.tag))
			&& (src_instr.ALU.srcA.reg.regId == src_instr.ALU.srcB.reg.regId)) {
				ret = encodeSrcReg(src_instr.ALU.srcA.reg);
			} else {
				breakpoint  // case not handled yet
			}

			break;
  }

	if (ret.get() == nullptr && !is_none) {
	  fprintf(stderr, "QPULib: missing case in encodeDestReg\n");
		assert(false);
	}

	return ret;
}


void setDestReg(QPULib::Instr const &src_instr, QPULib::v3d::instr::Instr &dst_instr) {
	std::unique_ptr<Location> ret = encodeDestReg(src_instr);
	if (ret.get() == nullptr) {
		breakpoint
		return;
	}

	if (src_instr.isMul()) {
		dst_instr.alu.mul.waddr = ret->to_waddr();
		dst_instr.alu.mul.output_pack = ret->output_pack();
	} else {
		dst_instr.alu.add.waddr = ret->to_waddr();
		dst_instr.alu.add.output_pack = ret->output_pack();
	}
}

/**
 * For v3d, the QPU and ELEM num are not special registers but instructions.
 *
 * In order to not disturb the code translation too much, they are derived from the target instructions:
 *
 *    mov(ACC0, QPU_ID)   // vc4: QPU_NUM  or SPECIAL_QPU_NUM
 *    mov(ACC0, ELEM_ID)  // vc4: ELEM_NUM or SPECIAL_ELEM_NUM
 *
 * This is the **only** operation in which they can be used.
 * This function checks for proper usage.
 * These special cases get translated to `tidx(r0)` and `eidx(r0)` respectively, as a special case
 * for A_BOR.
 *
 * If the check fails, a fatal exception is thrown.
 *
 * ==================================================================================================
 *
 * * Both these instructions use r0 here; this might produce conflicts with other instructions
 *   I haven't found a decent way yet to compensate for this.
 */
void checkSpecialIndex(QPULib::Instr const &src_instr) {
  if (src_instr.tag != ALU) {
		return;  // no problem here
	}

	auto srca = src_instr.ALU.srcA;
	auto srcb = src_instr.ALU.srcB;

	bool a_is_elem_num = (srca.tag == REG && srca.reg.tag == SPECIAL && srca.reg.regId == SPECIAL_ELEM_NUM);
	bool a_is_qpu_num  = (srca.tag == REG && srca.reg.tag == SPECIAL && srca.reg.regId == SPECIAL_QPU_NUM);
	bool b_is_elem_num = (srcb.tag == REG && srcb.reg.tag == SPECIAL && srcb.reg.regId == SPECIAL_ELEM_NUM);
	bool b_is_qpu_num  = (srcb.tag == REG && srcb.reg.tag == SPECIAL && srcb.reg.regId == SPECIAL_QPU_NUM);
	bool a_is_special  = a_is_elem_num || a_is_qpu_num;
	bool b_is_special  = b_is_elem_num || b_is_qpu_num;

	if (!a_is_special && !b_is_special) {
		return;  // Nothing to do
	}

	if (src_instr.ALU.op != A_BOR) {
		// All other instructions disallowed
		fatal("For v3d, special registers QPU_NUM and ELEM_NUM can only be used in a move instruction");
		return;
	}

	assertq((a_is_special && b_is_special), "src a and src b must both be special for QPU and ELEM nums");
	assertq(srca == srcb, "checkSpecialIndex(): src a and b must be the same if they are both special num's");
}


/**
 * Pre: `checkSpecialIndex()` has been called
 */
bool is_special_index(QPULib::Instr const &src_instr, Special index ) {
	assert(index == SPECIAL_ELEM_NUM || SPECIAL_QPU_NUM);

  if (src_instr.tag != ALU) {
		return false;
	}

	if (src_instr.ALU.op != A_BOR) {
		return false;
	}

	auto srca = src_instr.ALU.srcA;
	auto srcb = src_instr.ALU.srcB;
	bool a_is_special = (srca.tag == REG && srca.reg.tag == SPECIAL && srca.reg.regId == index);
	bool b_is_special = (srcb.tag == REG && srcb.reg.tag == SPECIAL && srcb.reg.regId == index);

	return (a_is_special && b_is_special);
}


/**
 */
void setCondTag(AssignCond cond, v3d::Instr &out_instr) {
	if (cond.is_always()) {
		return;
	}
	assert(cond.tag != AssignCond::Tag::NEVER);  // Not expecting this (yet)
	assert(cond.tag == AssignCond::Tag::FLAG);   // The only remaining option

	// NOTE: condition tags are set for add alu only here
	// TODO: Set for mul tag as well if required
	//       Prob the easiest is to always set them for both for now

	switch(cond.flag) {
		case ZS:
		case NS:
			out_instr.ifa(); 
			break;
		case ZC:
		case NC:
			out_instr.ifna(); 
			break;
		default:  assert(false);
	}

}


void setCondTag(AssignCond cond, Instructions &ret) {
	for (auto &instr : ret) {
		setCondTag(cond, instr);
	}
}


void handle_condition_tags(QPULib::Instr const &src_instr, Instructions &ret) {
	auto &cond = src_instr.ALU.cond;

	// src_instr.ALU.cond.tag has 3 possible values: NEVER, ALWAYS, FLAG
	assertq(cond.tag != AssignCond::Tag::NEVER, "NEVER encountered in ALU.cond.tag", true);             // Not expecting it
	assertq(cond.tag == AssignCond::Tag::FLAG || cond.is_always(), "Really expecting FLAG here", true); // Pedantry

	if (src_instr.ALU.setFlags) {
		// Set a condition flag with current instruction
		assertq(cond.is_always(), "Currently expecting only ALWAYS here", true);

		// Note that the condition is only set for the last in the list.
		// Any preceding instructions are assumed to be for calculating the condition
		Instr &instr = ret.back();

		assert(src_instr.ALU.setCond == Z || src_instr.ALU.setCond == N); 
		if (src_instr.ALU.setCond == Z) {
			instr.pushz();
		} else {
			instr.pushn();
		}

	} else {
		// use flag as run condition for current instruction(s)
		if (cond.is_always()) {
			return; // ALWAYS executes always (duh, is default)
		}

		setCondTag(cond, ret);
	}
}


bool translateOpcode(QPULib::Instr const &src_instr, Instructions &ret) {
	bool did_something = true;

	auto reg_a = src_instr.ALU.srcA;
	auto reg_b = src_instr.ALU.srcB;

	auto dst_reg = encodeDestReg(src_instr);

	if (dst_reg && reg_a.tag == REG && reg_b.tag == REG) {
		checkSpecialIndex(src_instr);
		if (is_special_index(src_instr, SPECIAL_QPU_NUM)) {
			ret << tidx(*dst_reg);
		} else if (is_special_index(src_instr, SPECIAL_ELEM_NUM)) {
			ret << eidx(*dst_reg);
		} else {
			auto src_a = encodeSrcReg(reg_a.reg);
			auto src_b = encodeSrcReg(reg_b.reg);
			assert(src_a && src_b);

			switch (src_instr.ALU.op) {
				case A_ADD:   ret << add(*dst_reg, *src_a, *src_b);          break;
				case A_SUB:   ret << sub(*dst_reg, *src_a, *src_b);          break;
				case A_BOR:   ret << bor(*dst_reg, *src_a, *src_b);          break;
				case M_FMUL:  ret << nop().fmul(*dst_reg, *src_a, *src_b);   break;
				case M_MUL24: ret << nop().smul24(*dst_reg, *src_a, *src_b); break;
				case A_FSUB:  ret << fsub(*dst_reg, *src_a, *src_b);         break;
				case A_FADD:  ret << fadd(*dst_reg, *src_a, *src_b);         break;
				case A_MIN:   ret << min(*dst_reg, *src_a, *src_b);          break;
				case A_MAX:   ret << max(*dst_reg, *src_a, *src_b);          break;
				case A_EIDX:  ret << eidx(*dst_reg);                         break;  // Note src reg's unused
				default:
					breakpoint  // unimplemented op
					did_something = false;
				break;
			}
		}
	} else if (dst_reg && reg_a.tag == REG && reg_b.tag == IMM) {
		auto src_a = encodeSrcReg(reg_a.reg);
		assert(src_a);
		SmallImm imm = encodeSmallImm(reg_b);

		switch (src_instr.ALU.op) {
			case A_SHL:   ret << shl(*dst_reg, *src_a, imm);          break;
			case A_SHR:   ret << shr(*dst_reg, *src_a, imm);          break;
			case A_ASR:   ret << asr(*dst_reg, *src_a, imm);          break;
			case A_BAND:  ret << band(*dst_reg, *src_a, imm);         break;
			case A_SUB:   ret << sub(*dst_reg, *src_a, imm);          break;
			case A_ADD:   ret << add(*dst_reg, *src_a, imm);          break;
			case M_FMUL:  ret << nop().fmul(*dst_reg, *src_a, imm);   break;
			case M_MUL24: ret << nop().smul24(*dst_reg, *src_a, imm); break;
			case A_ItoF:  ret << itof(*dst_reg, *src_a, imm);         break;
			case A_FtoI:  ret << ftoi(*dst_reg, *src_a, imm);         break;
			default:
				breakpoint  // unimplemented op
				did_something = false;
			break;
		}
	} else if (dst_reg && reg_a.tag == IMM && reg_b.tag == REG) {
		SmallImm imm = encodeSmallImm(reg_a);
		auto src_b = encodeSrcReg(reg_b.reg);
		assert(src_b);

		switch (src_instr.ALU.op) {
			case M_MUL24: ret << nop().smul24(*dst_reg, imm, *src_b); break;
			case M_FMUL:  ret << nop().fmul(*dst_reg, imm, *src_b);   break;
			case A_FSUB:  ret << fsub(*dst_reg, imm, *src_b);         break;
			case A_SUB:   ret << sub(*dst_reg, imm, *src_b);          break;
			default:
				breakpoint  // unimplemented op
				did_something = false;
			break;
		}
	} else if (dst_reg && reg_a.tag == IMM && reg_b.tag == IMM) {
		SmallImm imm_a = encodeSmallImm(reg_a);
		SmallImm imm_b = encodeSmallImm(reg_b);

		switch (src_instr.ALU.op) {
			case A_BOR:   ret << bor(*dst_reg, imm_a, imm_b);          break;
			default:
				breakpoint  // unimplemented op
				did_something = false;
			break;
		}
	} else {
		breakpoint  // Unhandled combination of inputs/output
		did_something = false;
	}

	return did_something;
}


/**
 * @return true if rotate handled, false otherwise
 */
bool translateRotate(QPULib::Instr const &instr, Instructions &ret) {
	if (instr.ALU.op != M_ROTATE) {
		return false;
	}

	// dest is location where r1 (result of rotate) must be stored 
	auto dst_reg = encodeDestReg(instr);
	assert(dst_reg);
	assert(dst_reg->to_mux() != V3D_QPU_MUX_R1);  // anything except dest of rotate

	auto reg_a = instr.ALU.srcA;
	auto src_a = encodeSrcReg(reg_a.reg);         // Must be r0, checked in rotate()
	auto reg_b = instr.ALU.srcB;                  // reg b is either r5 or small imm

	// TODO: the Target source step already adds a nop. Check and add a nop here conditionally
	ret << nop().comment("required for rotate");

	if (reg_b.tag == REG) {
		breakpoint

		assert(instr.ALU.srcB.reg.tag == ACC && instr.ALU.srcB.reg.regId == 5);  // reg b must be r5
		auto src_b = encodeSrcReg(reg_b.reg);

		ret << rotate(r1, *src_a, *src_b);

	} else if (reg_b.tag == IMM) {
		SmallImm imm = encodeSmallImm(reg_b);  // Legal values small imm tested in rotate()

		ret << rotate(r1, *src_a, imm);
	} else {
		breakpoint  // Unhandled combination of inputs/output
	}

	ret << bor(*dst_reg, r1, r1);

	return true;
}


Instructions encodeLoadImmediate(QPULib::Instr full_instr) {
	assert(full_instr.tag == LI);
	auto &instr = full_instr.LI;
	auto dst = encodeDestReg(full_instr);

	Instructions ret;
	int rep_value;

	std::string err_label;
	std::string err_value;

	if (instr.imm.tag == IMM_INT32) {
		int value = instr.imm.intVal;
		int left_shift = 0;

		while (value != 0 && (value & 1) == 0) {
			left_shift++;
			value >>= 1;
		}

		if (SmallImm::int_to_opcode_value(value, rep_value)) {
			SmallImm imm(rep_value);
			ret << mov(*dst, imm);

			if (left_shift > 0) {
				ret << shl(*dst, *dst, SmallImm(left_shift));
			}

		} else {
			// TODO: figure out how to handle other int immediates, if necessary at all
			err_label = "int";
			err_value = std::to_string(value);
		}

	} else if (instr.imm.tag == IMM_FLOAT32) {
		float value = instr.imm.floatVal;

		if (SmallImm::float_to_opcode_value(value, rep_value)) {
			ret << nop().fmov(*dst, rep_value);  // TODO perhaps make 2nd param Small Imm
		} else {
			// TODO: figure out how to handle other float immediates, if necessary at all
			err_label = "float";
			err_value = std::to_string(value);
		}
	} else if (instr.imm.tag == IMM_MASK) {
		debug_break("encodeLoadImmediate(): IMM_MASK not handled yet");
	} else {
		debug_break("encodeLoadImmediate(): unknown tag value");
	}

	if (!err_value.empty()) {
		assert(!err_label.empty());

		std::string str = "LI: Can't handle ";
		str += err_label + " value '" + err_value;
		str += "' as small immediate";

		breakpoint
		local_errors << str;
		ret << nop().comment(str);
	}


	if (instr.setFlags) {
		breakpoint;  // to check what flags need to be set - case not handled yet
	}

	setCondTag(instr.cond, ret);
	return ret;
}


Instructions encodeALUOp(QPULib::Instr instr) {
	Instructions ret;

	if (instr.isUniformLoad()) {
		Reg dst_reg = instr.ALU.dest;
		uint8_t rf_addr = to_waddr(dst_reg);
		ret << ldunifrf(rf_addr);
 	} else if (translateRotate(instr, ret)) {
		handle_condition_tags(instr, ret);
	} else if (translateOpcode(instr, ret)) {
		handle_condition_tags(instr, ret);
	} else {
		assertq(false, "Missing translate operation for ALU instruction", true);  // Something missing, check
	}

	assert(!ret.empty());
	return ret;
}


/**
 * Convert conditions from Target source to v3d
 *
 * Incoming conditions are vc4 only, the conditions don't exist on v3d.
 * They therefore need to be translated.
 */
void encodeBranchCondition(v3d::instr::Instr &dst_instr, QPULib::BranchCond src_cond) {
	// TODO How to deal with:
	//
	//      dst_instr.na0();  // Verified correct
	//      dst_instr.a0();  // TODO: verify

	if (src_cond.tag == COND_ALL) {
		switch (src_cond.flag) {
			case ZC:
			case NC:
				dst_instr.allna();
				break;
			case ZS:
			case NS:
				dst_instr.alla();
				break;
			default:
				debug_break("Unknown branch condition under COND_ALL");  // Warn me if this happens
		}
	} else if (src_cond.tag == COND_ANY) {
		switch (src_cond.flag) {
			case ZC:
			case NC:
				dst_instr.anyna();  // TODO: verify
				break;
			case ZS:
			case NS:
				dst_instr.anya();  // TODO: verify
				break;
			default:
				debug_break("Unknown branch condition under COND_ANY");  // Warn me if this happens
		}
	} else {
		debug_break("Branch condition not COND_ALL or COND_ANY");  // Warn me if this happens
	}
}


/**
 * Create a branch instruction, including any branch conditions,
 * from Target source instruction.
 */
v3d::instr::Instr encodeBranchLabel(QPULib::Instr src_instr) {
	assert(src_instr.tag == BRL);
	auto &instr = src_instr.BRL;

	// Prepare as branch without offset but with label
	auto dst_instr = branch(0, true);
	dst_instr.label(instr.label);
	encodeBranchCondition(dst_instr, instr.cond);

	return dst_instr;
}


/**
 * Convert intermediate instruction into core instruction.
 *
 * **Pre:** All instructions not meant for v3d are detected beforehand and flagged as error.
 */
Instructions encodeInstr(QPULib::Instr instr) {
	Instructions ret;
	bool no_output = false;

  // Encode core instruction
  switch (instr.tag) {
		//
		// Unhandled tags - ignored or should have been handled beforehand
		//
    case BR:
			 assertq(false, "Not expecting BR any more, branch creation now goes with BRL", true);
		break;

    case INIT_BEGIN:
    case INIT_END:
    case END:         // vc4 end program marker
			assertq(false, "Not expecting INIT or END tag here", true);
		break;

    // Print instructions - ignored
    case PRI:
    case PRS:
    case PRF:
			no_output = true;
		break;

		//
		// Label handling
		//
		case LAB: {
			// create a label meta-instruction
			Instr n;
			n.is_label(true);
			n.label(instr.label());
			
			ret << n;
		}
		break;

		case BRL: {
			// Create branch instruction with label
			ret << encodeBranchLabel(instr);
		}
		break;

		//
		// Handled tags
		//
    case LI:           ret << encodeLoadImmediate(instr); break;
    case ALU:          ret << encodeALUOp(instr);         break;
    case TMU0_TO_ACC4: ret << nop().ldtmu(r4);            break;
		case NO_OP:        ret << nop();                      break;
		case TMUWT:        ret << tmuwt();                    break;

		default:
  		fatal("v3d: missing case in encodeInstr");
  }

	assert(no_output || !ret.empty());

	if (!ret.empty()) {
		if (!instr.header().empty()) {
			ret.front().header(instr.header());
		}

		if (!instr.comment().empty()) {
			ret.front().comment(instr.comment());
		}
	}

	return ret;
}


/**
 * This is where standard initialization code can be added.
 *
 * Called;
 * - after code for loading uniforms has been encoded
 * - any other target initialization code has been added
 * - before the encoding of the main body.
 *
 * Serious consideration: Any regfile registers used in the generated code here,
 * have not participated in the liveness determination. This may lead to screwed up variable assignments.
 * **Keep this in mind!**
 */
Instructions encode_init(uint8_t numQPUs) {
	Instructions ret;

/*
	    ret << calc_offset(numQPUs, slots.get_temp_slot());  // offset in r0

			// Add offset to uniforms that are addresses
			if (!tmu_regs.empty()) {  // Theoretically possible, not gonna happen IRL
			                          // If there would be no addresses, we also wouln't need to calc offset and stride

				const char *text = "# Set offsets for uniform addresses";

				// For multiple adds here, it would be possible to use alu and mul in parallel,
				// as happens in the summation kernel.

				bool is_first = true;
				for (auto n : tmu_regs) {
					auto instr = add(rf(n), rf(n), r0);

					if (is_first) {
						instr.comment(text);
						is_first = false;
					}

					ret << instr;
				}
			}

			ret << calc_stride(numQPUs, slots.get_slot());
*/
	ret //<< set_qpu_id(0)           - done in target code
	    //<< set_qpu_num(numQPUs, 1) - passed in
	    << instr::enable_tmu_read();

	return ret;
}


/**
 * Check assumption: uniform loads are always at the top of the instruction list.
 */
bool checkUniformAtTop(Seq<QPULib::Instr> &instrs) {
	bool doing_top = true;

  for (int i = 0; i < instrs.numElems; i++) {
    QPULib::Instr instr = instrs.elems[i];
		if (doing_top) {
			if (instr.isUniformLoad()) {
				continue;  // as expected
			}

			doing_top = false;
		} else {
			if (!instr.isUniformLoad()) {
				continue;  // as expected
			}

			return false;  // Encountered uniform NOT at the top of the instruction list
		}
	}

	return true;
}


/**
 * Translate instructions from target to v3d
 */
void _encode(uint8_t numQPUs, Seq<QPULib::Instr> &instrs, Instructions &instructions) {
	assert(checkUniformAtTop(instrs));
	bool prev_was_init_begin = false;
	bool prev_was_init_end    = false;

	// Main loop
  for (int i = 0; i < instrs.numElems; i++) {
    QPULib::Instr instr = instrs.elems[i];
		check_instruction_tag_for_platform(instr.tag, false);

		if (instr.tag == INIT_BEGIN) {
			prev_was_init_begin = true;
		} else if (instr.tag == INIT_END) {
			instructions << encode_init(numQPUs);
			prev_was_init_end = true;
		} else {
			auto ret = v3d::encodeInstr(instr);

			if (prev_was_init_begin) {
				ret.front().header("Init block");
				prev_was_init_begin = false;
			}

			if (prev_was_init_end) {
				ret.front().header("Main program");
				prev_was_init_end = false;
			}

			instructions << ret;
		}
  }

	instructions << sync_tmu()
			         << end_program();
}

}  // anon namespace


///////////////////////////////////////////////////////////////////////////////
// Class KernelDriver
///////////////////////////////////////////////////////////////////////////////

KernelDriver::KernelDriver() : QPULib::KernelDriver(V3dBuffer) {}


void KernelDriver::encode(int numQPUs) {
	if (instructions.size() > 0) return;  // Don't bother if already encoded

	if (numQPUs != 1 && numQPUs != 8) {
		errors << "Num QPU's must be 1 or 8";
		return;
	}
	local_numQPUs = (uint8_t) numQPUs;

	// Encode target instructions
	_encode((uint8_t) numQPUs, m_targetCode, instructions);
	removeLabels(instructions);

	if (!local_errors.empty()) {
		breakpoint
	}

	errors << local_errors;
	local_errors.clear();
}


/**
 * Generate the opcodes for the currrent v3d instruction sequence
 */
std::vector<uint64_t> KernelDriver::to_opcodes() {
	assert(instructions.size() > 0);

	std::vector<uint64_t> code;  // opcodes for v3d

	for (auto const &inst : instructions) {
		code << inst.code();
	}

	return code;
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t> *params) {

	// Assumption: code in a kernel, once allocated, doesn't change
	if (qpuCodeMem.allocated()) {
		assert(instructions.size() > 0);
		assert(instructions.size() >= qpuCodeMem.size());  // Tentative check, not perfect
		                                                   // actual opcode seq can be smaller due to removal labels
	} else {
		std::vector<uint64_t> code = to_opcodes();

		// Allocate memory for the QPU code
		qpuCodeMem.alloc(code.size());
		qpuCodeMem.copyFrom(code);  // Copy kernel to code memory

		qpuCodeMemOffset = 8*code.size();  // TODO check if correct
	}

	// Allocate memory for the parameters if not done already
	// TODO Not used in v3d, do we need this?
	int numWords = (12*MAX_KERNEL_PARAMS + 12*2);
	if (paramMem.allocated()) {
		assert(paramMem.size() == numWords);
	} else {
		paramMem.alloc(numWords);
	}

	v3d::invoke(numQPUs, qpuCodeMem, qpuCodeMemOffset, params);
}


void KernelDriver::emit_opcodes(FILE *f) {
	fprintf(f, "Opcodes for v3d\n");
	fprintf(f, "===============\n\n");

	if (instructions.empty()) {
		fprintf(f, "<No opcodes to print>\n");
	} else {
		for (auto const &instr : instructions) {

			if (!instr.header().empty()) {
				fprintf(f, "\n# %s\n", instr.header().c_str());
			}

			if (!instr.comment().empty()) {
				fprintf(f, "%s  # %s\n", instr.mnemonic().c_str(), instr.comment().c_str());
			} else {
				fprintf(f, "%s\n", instr.mnemonic().c_str());
			}
		}
	}

	fprintf(f, "\n");
	fflush(f);
}

}  // namespace v3d
}  // namespace QPULib

#endif  // QPU_MODE
