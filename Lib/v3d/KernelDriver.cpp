/**
 * Collected comments for creating opcodes
 *
 *     uint32_t cond = encodeAssignCond(instr.LI.cond) << 17;
 *
 * LI:
 *     uint32_t sig   = ((instr.hasImm() || instr.isRot) ? 13 : 1) << 28;
 *     uint32_t cond  = encodeAssignCond(instr.ALU.cond) << (instr.isMul() ? 14 : 17);
 *     uint32_t ws;  // bitfield that selects between regfile A and B
 *			              // There is no such distinction on v3d, there is one regfile
 *     uint32_t sf    = (instr.ALU.setFlags ? 1 : 0) << 13;
 *     *high          = sig | cond | ws | sf | waddr_add | waddr_mul;
 *
 * ALU:
 *     uint32_t waddr_add = 39 << 6;
 *     uint32_t waddr_mul = 39;
 *     uint32_t sig = 0xe8000000;
 *     uint32_t incOrDec = (instr.tag == SINC ? 0 : 1) << 4;
 *     *high = sig | waddr_add | waddr_mul;
 *     *low = incOrDec | instr.semaId;
 */
#include "KernelDriver.h"
#include <memory>
#include <iostream>
#include "../Support/basics.h"
#include "Target/SmallLiteral.h"  // decodeSmallLit()
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
 * @param opcodes  current list of output instruction, out-parameter.
 *                 Passed in because some extra instruction may be needed
 *                 for v3d.
 */
std::unique_ptr<Location> encodeSrcReg(Reg reg, Instructions &opcodes) {
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
        case SPECIAL_UNIFORM:
					assert(false);  // Not expecting this, handled before this call
				break;
        case SPECIAL_ELEM_NUM:
					warning("SPECIAL_ELEM_NUM needs a way to select a register");
					opcodes << eidx(r0);
					ret.reset(new Register(r0));  // register should be selected on usage in code
				break;
        case SPECIAL_QPU_NUM: {
					warning("SPECIAL_QPU_NUM needs a way to select a register");
					//Register tmp_reg = r1;  // register should be selected on usage in code
					//opcodes << get_num_qpus(tmp_reg, local_numQPUs);
					//ret.reset(new Register(tmp_reg));

					opcodes << tidx(r0).comment("Set QPU id SPECIAL_QPU_NUM", true);
					ret.reset(new Register(r0));
				}
				break;

				// Not handled (yet)
        case SPECIAL_VPM_READ: // return 48;
					breakpoint
				break;
        case SPECIAL_DMA_LD_WAIT:
					breakpoint
          // // in REG_A
					// return 50;
				break;
        case SPECIAL_DMA_ST_WAIT:
					breakpoint
          // // in REG_B;
					// return 50;
				break;
      }
			break;
    case NONE:
			is_none = true;
			breakpoint
			break;
  }

	if (ret.get() == nullptr && !is_none) {
	  fprintf(stderr, "QPULib: missing case in encodeSrcReg\n");
breakpoint
		assert(false);
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
				Instructions dummy;
				ret = encodeSrcReg(src_instr.ALU.srcA.reg, dummy);
				assert(dummy.empty());
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


bool translateOpcode(QPULib::Instr const &src_instr, Instructions &ret) {
	bool did_something = true;

	auto reg_a = src_instr.ALU.srcA;
	auto reg_b = src_instr.ALU.srcB;

	auto dst_reg = encodeDestReg(src_instr);

	if (dst_reg && reg_a.tag == REG && reg_b.tag == REG) {
		auto src_a = encodeSrcReg(reg_a.reg, ret);
		auto src_b = encodeSrcReg(reg_b.reg, ret);
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
			default:
				breakpoint  // unimplemented op
				did_something = false;
			break;
		}
	} else if (dst_reg && reg_a.tag == REG && reg_b.tag == IMM) {
		auto src_a = encodeSrcReg(reg_a.reg, ret);
		assert(src_a);
		SmallImm imm = encodeSmallImm(reg_b);

		switch (src_instr.ALU.op) {
			case A_SHL:  ret << shl(*dst_reg, *src_a, imm);        break;
			case A_SUB:  ret << sub(*dst_reg, *src_a, imm);        break;
			case A_ADD:  ret << add(*dst_reg, *src_a, imm);        break;
			case M_FMUL: ret << nop().fmul(*dst_reg, *src_a, imm); break;
			case A_ItoF: ret << itof(*dst_reg, *src_a, imm);       break;
			case A_FtoI: ret << ftoi(*dst_reg, *src_a, imm);       break;
			default:
				breakpoint  // unimplemented op
				did_something = false;
			break;
		}
	} else if (dst_reg && reg_a.tag == IMM && reg_b.tag == REG) {
		SmallImm imm = encodeSmallImm(reg_a);
		auto src_b = encodeSrcReg(reg_b.reg, ret);
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
	auto src_a = encodeSrcReg(reg_a.reg, ret);    // Must be r0, checked in rotate()
	auto reg_b = instr.ALU.srcB;                  // reg b is either r5 or small imm

	if (reg_b.tag == REG) {
		breakpoint

		assert(instr.ALU.srcB.reg.tag == ACC && instr.ALU.srcB.reg.regId == 5);  // reg b must be r5
		auto src_b = encodeSrcReg(reg_b.reg, ret);

		ret << nop().comment("required for rotate", true)
		    << rotate(r1, *src_a, *src_b)
		    << bor(*dst_reg, r1, r1)
		;

	} else if (reg_b.tag == IMM) {
		assert(-15 <= reg_b.smallImm.val && reg_b.smallImm.val <= 16); // smallimm must be in proper range
		                                                               // Also tested in rotate()
		SmallImm imm = encodeSmallImm(reg_b);

		ret << nop().comment("required for rotate", true)
		    << rotate(r1, *src_a, imm)
		    << bor(*dst_reg, r1, r1)
		;
	} else {
		breakpoint  // Unhandled combination of inputs/output
	}

	return true;
}


Instructions encodeLoadImmediate(QPULib::Instr instr) {
	assert(instr.tag == LI);

	Instructions ret;
	int rep_value;

	if (!SmallImm::to_opcode_value((float) instr.LI.imm.intVal, rep_value)) {
		// TODO: figure out how to handle large immediates, if necessary at all
		std::string str = "LI: Can't handle value '";
		str += std::to_string(instr.LI.imm.intVal);
		str += "'as small immediate";

		local_errors << str;
		ret << nop().comment(str, true);
		return ret;
	}

	if (instr.LI.setFlags) {
		breakpoint;  // to check what flags need to be set - case not handled yet
	}

	auto dst = encodeDestReg(instr);
	SmallImm imm(rep_value);
	auto out_instr = mov(*dst, imm);

	if (instr.LI.cond.tag != ALWAYS) {  // ALWAYS executes always (duh)
		//
		// qpu_instr.h, line 74, enum v3d_qpu_uf:
		//
		// How I interpret this:
		//  - AND: if all bits set
		//  - NOR: if no bits set
		//  - N  : field not set
		//  - Z  : Field zero
		//  - N  : field negative set
		//  - C  : Field negative cleared
		//
		// What the bits are is not clear at this point.
		// These assumptions are probably wrong, but I need a starting point.
		// TODO: make tests to verify these assumptions (how? No clue right now)
		// ------------------------------------------------------------------------
		// So:
		// - vc4 `if all(nc)...` -> ANDC
		//
		// vc4 `nc` - negative clear, ie. >= 0
		// vc4 `ns` - negative set,   ie.  < 0
		//
		switch (instr.LI.cond.flag) {
			// TODO test what happens here, for vc4 as well as v3d
			//      v3d flags used are prob wrong!
			// TODO how to distinguish here between add and mul ALU?
			case NC: out_instr.andc();  break;
			case NS: out_instr.andnc(); break;
			default:
				breakpoint;        // check,  case not handled yet
		}
	}

	ret << out_instr;
	return ret;
}


/**
 * Convert intermediate instruction into core instruction
 */
Instructions encodeInstr(QPULib::Instr instr) {
	Instructions ret;
	bool no_output = false;

  switch (instr.tag) {
    case IRQ:
			assert(false);  // Not wanting this
      break;
    case DMA_LOAD_WAIT:
    case DMA_STORE_WAIT: {
			assert(false);  // Not wanting this
      break;
    }
	}

  // Encode core instruction
  switch (instr.tag) {
    case LI:  // Load immediate
			ret << encodeLoadImmediate(instr); 
		break;

    case BR: { // Branch
			//breakpoint  // TODO examine
      assert(!instr.BR.target.useRegOffset);  // Register offset not yet supported

			ret << branch(instr.BR.target.immOffset, instr.BR.target.relative);

			// TODO: Figure out how to deal with branch conditions
			//       The call below is vc4 only, the conditions don't exist on v3d
			//
			// See 'Condition Codes' in the vc4 ref guide
/*
      uint32_t cond = encodeBranchCond(instr.BR.cond) << 20;
*/
    }
		break;

    // ALU
    case ALU: {
			if (instr.isUniformLoad()) {
					Reg dst_reg = instr.ALU.dest;
					uint8_t rf_addr = to_waddr(dst_reg);
					QPULib::v3d::instr::Instr instr = ldunifrf(rf_addr);
					ret << instr;
					break;
      } else if (translateRotate(instr, ret)) {
				break;  // all is well
      } else if (translateOpcode(instr, ret)) {
				break; // All is well
      } else {
				breakpoint  // Something missing, check
				assert(false);
			}
    }
		break;

    case END:
			assert(false);  // vc4 end program marker, should not receive this
		break;

    case TMU0_TO_ACC4: {
			ret << nop().ldtmu(r4);  // NOTE: added by WR
    }
		break;

    // Semaphore increment/decrement
    case SINC:
    case SDEC: {
			assert(false);  // TODO examine
    }
		break;

    // No-op instruction
    case NO_OP:
			ret << nop();
			break;

    // Print instructions - ignored
    case PRI:
    case PRS:
    case PRF:
			no_output = true;
		break;

		default:
  		fatal("v3d: missing case in encodeInstr");
  }

	assert(no_output || !ret.empty());
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
	bool did_init = false;

	// Main loop
  for (int i = 0; i < instrs.numElems; i++) {
    QPULib::Instr instr = instrs.elems[i];

		// Assumption: uniform loads are always at the top of the instruction list
		bool doing_init = !did_init && !instr.isUniformLoad();
		if (doing_init) {
			instructions << encode_init(numQPUs);
			did_init = true;
		}
	
		auto ret = v3d::encodeInstr(instr);

		if (doing_init) {
			ret.front().comment("Main program");
		}

		instructions << ret;
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

	if (!local_errors.empty()) {
		breakpoint
	}

	errors << local_errors;
	local_errors.clear();
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t> *params) {
	debug("Called v3d KernelDriver::invoke()");
	assert(instructions.size() > 0);

	// Assumption: code in a kernel, once allocated, doesn't change
	if (qpuCodeMem.allocated()) {
		assert(instructions.size() == qpuCodeMem.size());  // Tentative check, not perfect
	} else {
		std::vector<uint64_t> code;  // opcodes for v3d
		for (auto const &inst : instructions) {
			code << inst.code();
		}

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
			if (!instr.comment().empty()) {
				if (instr.is_side_comment()) {
					fprintf(f, "%s  # %s\n", instr.mnemonic().c_str(), instr.comment().c_str());
				} else {
					fprintf(f, "\n# %s\n", instr.comment().c_str());
					fprintf(f, "%s\n", instr.mnemonic().c_str());
				}
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
