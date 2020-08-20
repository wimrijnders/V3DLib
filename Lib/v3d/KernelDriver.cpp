#include "KernelDriver.h"
#include <memory>
#include <iostream>
#include "../Support/basics.h"
#include "Invoke.h"
#include "instr/Instr.h"
#include "instr/Snippets.h"

#ifdef QPU_MODE

using namespace QPULib::v3d::instr;
using std::cout;
using std::endl;
using OpCode  = QPULib::v3d::instr::Instr;
using OpCodes = std::vector<OpCode>;

namespace QPULib {
namespace v3d {
using namespace QPULib::v3d::instr;

using OpCode  = QPULib::v3d::instr::Instr;
using OpCodes = std::vector<OpCode>;

static int local_numQPUs = 0;
std::vector<std::string> local_errors;

/**
 * source:  https://github.com/Idein/py-videocore6/blob/3c407a2c0a3a0d9d56a5d0953caa7b0a4e92fa89/examples/summation.py#L22
 */
static OpCodes get_num_qpus(Register const &reg) {
	assert(local_numQPUs == 1 || local_numQPUs == 8);
	assert(reg.is_dest_acc());

	OpCodes ret;

	if (local_numQPUs == 1) {
		ret << mov(reg, 0);
	} else { //  num_qpus == 8
		breakpoint
		ret << tidx(reg)
		    << shr(reg, reg, 2)
		    << band(reg, reg, 0b1111);
	}

	return ret;
}



uint8_t const REGB_OFFSET = 32;
uint8_t const NOP_ADDR    = 39;

v3d_qpu_mul_op encodeMulOp(ALUOp in_op) {
	v3d_qpu_mul_op op;

  switch (in_op) {
    case NOP:    op = V3D_QPU_M_NOP;  break;
    case M_FMUL: op = V3D_QPU_M_FMUL; break;
    case M_MUL24: // No clue
    case M_V8MUL:
    case M_V8MIN:
    case M_V8MAX:
    case M_V8ADDS:
    case M_V8SUBS:
		default:
  		fprintf(stderr, "QPULib: unknown mul op\n");
			assert(false);
  }

/*
 Other possible values:

        V3D_QPU_M_ADD,
        V3D_QPU_M_SUB,
        V3D_QPU_M_UMUL24,
        V3D_QPU_M_VFMUL,
        V3D_QPU_M_SMUL24,
        V3D_QPU_M_MULTOP,
        V3D_QPU_M_FMOV,
        V3D_QPU_M_MOV,
*/

	return op;
}


v3d_qpu_add_op encodeAddOp(ALUOp in_op) {
	v3d_qpu_add_op op;

	// Other possible values: See `enum v3d_qpu_add_op` in `mesa/src/broadcom/qpu/qpu_instr.h`
  switch (in_op) {
    case NOP   : op = V3D_QPU_A_NOP;   break;
    case A_FADD: op = V3D_QPU_A_FADD;  break;
    case A_FSUB: op = V3D_QPU_A_FSUB;  break;
    case A_FMIN: op = V3D_QPU_A_FMIN;  break;
    case A_FMAX: op = V3D_QPU_A_FMAX;  break;
    case A_FtoI: op = V3D_QPU_A_FTOIN; break;  // Not 100% sure
    case A_ItoF: op = V3D_QPU_A_ITOF;  break;
    case A_ADD : op = V3D_QPU_A_ADD;   break;
    case A_SUB : op = V3D_QPU_A_SUB;   break;
    case A_SHR : op = V3D_QPU_A_SHR;   break;
    case A_ASR : op = V3D_QPU_A_ASR;   break;
    case A_ROR : op = V3D_QPU_A_ROR;   break;
    case A_SHL : op = V3D_QPU_A_SHL;   break;
    case A_MIN : op = V3D_QPU_A_MIN;   break;
    case A_MAX : op = V3D_QPU_A_MAX;   break;
    case A_BAND: op = V3D_QPU_A_AND;   break;  // Not 100% sure
    case A_BOR : op = V3D_QPU_A_OR;    break;  // Not 100% sure
    case A_BXOR: op = V3D_QPU_A_XOR;   break;  // Not 100% sure
    case A_BNOT: op = V3D_QPU_A_NOT;   break;  // Not 100% sure
    case A_CLZ : op = V3D_QPU_A_CLZ;   break;

    case A_FMINABS:  // No clue
    case A_FMAXABS:
    case A_V8ADDS:
    case A_V8SUBS:
		default:
  		fprintf(stderr, "QPULib: unknown mul op\n");
			assert(false);
  }

	return op;
}


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
/*
        case SPECIAL_RD_SETUP:    return 49;
        case SPECIAL_WR_SETUP:    return 49;
        case SPECIAL_DMA_LD_ADDR: return 50;
        case SPECIAL_HOST_INT:    return 38;
*/
        case SPECIAL_VPM_WRITE:           // Write TMY, to set data to write
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
			is_none = true;
			breakpoint
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
		return;
	}

	if (src_instr.isMul()) {
		breakpoint

		dst_instr.alu.mul.waddr = ret->to_waddr();
		dst_instr.alu.mul.output_pack = ret->output_pack();
	} else {
		dst_instr.alu.add.waddr = ret->to_waddr();
		dst_instr.alu.add.output_pack = ret->output_pack();
	}
}


/**
 * @param ret  current list of output instruction, out-parameter.
 *             Passed in because some extra instruction may be needed
 *             for v3d.
 */
std::unique_ptr<Location> encodeSrcReg(Reg reg, OpCodes &opcodes) {
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
					Register tmp_reg = r1;  // register should be selected on usage in code
					opcodes << get_num_qpus(tmp_reg);
					ret.reset(new Register(tmp_reg));
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
		assert(false);
	}

	return ret;
}


uint8_t encodeSrcReg_old(Reg reg) {
  switch (reg.tag) {
    case REG_A:
      assert(reg.regId >= 0 && reg.regId < 32);
      return (uint8_t) reg.regId;
    case REG_B:
      assert(reg.regId >= 0 && reg.regId < 32);
      return (uint8_t) (REGB_OFFSET + reg.regId);
    case ACC:
      assert(reg.regId >= 0 && reg.regId <= 4);
			assert(false); // TODO: set reg.regId
      //*mux = reg.regId;
			return 0;
    case NONE:
      return NOP_ADDR;
    case SPECIAL:
      switch (reg.regId) {
        case SPECIAL_UNIFORM:  return 32;
        case SPECIAL_ELEM_NUM: return 38;
        case SPECIAL_QPU_NUM:  return 38;
        case SPECIAL_VPM_READ: return 48;
        case SPECIAL_DMA_LD_WAIT:
          // in REG_A
					return 50;
        case SPECIAL_DMA_ST_WAIT:
          // in REG_B;
					return 50;
      }
  }
  fprintf(stderr, "QPULib: missing case in encodeSrcReg_old\n");
  exit(EXIT_FAILURE);
}


bool translateOpcode(QPULib::Instr const &src_instr, OpCodes &ret) {
	bool did_something = true;

	auto reg_a = src_instr.ALU.srcA;
	auto reg_b = src_instr.ALU.srcB;

	auto dst_reg = encodeDestReg(src_instr);

	auto src_a = encodeSrcReg(reg_a.reg, ret);
	auto src_b = encodeSrcReg(reg_b.reg, ret);

	switch (src_instr.ALU.op) {
		case A_SHL: {
			assert(dst_reg.get() != nullptr);
			assert(src_a.get() != nullptr);

			assert(reg_b.tag == IMM); 
			SmallImm imm(reg_b.smallImm.val);

			ret << shl(*dst_reg, *src_a, imm);
		}
		break;
		case A_ADD: {
			assert(dst_reg.get() != nullptr);
			assert(src_a.get() != nullptr);
			assert(src_b.get() != nullptr);
			ret << add(*dst_reg, *src_a, *src_b);
		}
		break;
		case A_SUB: {
			assert(dst_reg.get() != nullptr);
			assert(src_a.get() != nullptr);
			assert(src_b.get() != nullptr);
			ret << sub(*dst_reg, *src_a, *src_b);
		}
		break;
		case A_BOR: {
			assert(dst_reg.get() != nullptr);
			assert(src_a.get() != nullptr);
			assert(src_b.get() != nullptr);
			ret << bor(*dst_reg, *src_a, *src_b);
		}
		break;
		default:
			breakpoint  // To catch inimplemented opcodes
			did_something = false;
			break;
	}

	return did_something;
}


OpCodes encodeInstr(QPULib::Instr instr) {
	OpCodes ret;

  // Convert intermediate instruction into core instruction
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
    // Load immediate
    case LI: {
			int rep_value;

			if (instr.LI.setFlags) {
				breakpoint;  // to check what flags need to be set - case not handled yet
			}
			if (instr.LI.cond.tag != ALWAYS) {
				breakpoint;  // check,  case not handled yet
			}
			if (!SmallImm::to_opcode_value((float) instr.LI.imm.intVal, rep_value)) {
				// TODO: figure out how to handle large immediates
				std::string str = "Can't handle value '";
				str += std::to_string(instr.LI.imm.intVal);
				str += "'as small immediate";
				breakpoint
				local_errors << str;
				ret << nop();
			} else {
				auto dst = encodeDestReg(instr);
				SmallImm imm(rep_value);

				ret << mov(*dst, imm);
/*
      RegTag file;
      uint32_t cond = encodeAssignCond(instr.LI.cond) << 17;
      uint32_t sf   = (instr.LI.setFlags ? 1 : 0) << 13;
      *high         = 0xe0000000 | cond | ws | sf | waddr_add | waddr_mul;
*/
			}
    }
		break;

    // Branch
    case BR: {
			breakpoint  // TODO examine

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
			QPULib::v3d::instr::Instr ret_instr;  // Note inits to nop-nop
			if (!instr.isUniformLoad()) {
	      v3d::setDestReg(instr, ret_instr);
			}
/*
      uint32_t sig   = ((instr.hasImm() || instr.isRot) ? 13 : 1) << 28;
      uint32_t cond  = encodeAssignCond(instr.ALU.cond) << (instr.isMul() ? 14 : 17);
      uint32_t ws;  // bitfield that selects between regfile A and B
			              // There is no such distinction on v3d, there is one regfile
      uint32_t sf    = (instr.ALU.setFlags ? 1 : 0) << 13;
      *high          = sig | cond | ws | sf | waddr_add | waddr_mul;
*/
			if (instr.isUniformLoad()) {
					Reg dst_reg = instr.ALU.dest;
					uint8_t rf_addr = to_waddr(dst_reg);
					//if (rf_addr % REGB_OFFSET != 0) {
					//	breakpoint  // warn me if this happens
					//}
					ret_instr = ldunifrf(rf_addr);
					ret << ret_instr;
					break;
      } else if (translateOpcode(instr, ret)) {
				break; // All is well
      } else if (instr.ALU.op == M_ROTATE) {
				assert(false); // TODO
/*
        assert(instr.ALU.srcA.tag == REG && instr.ALU.srcA.reg.tag == ACC &&
               instr.ALU.srcA.reg.regId == 0);
        assert(instr.ALU.srcB.tag == REG ?
               instr.ALU.srcB.reg.tag == ACC && instr.ALU.srcB.reg.regId == 5
               : true);
        uint32_t mulOp = encodeMulOp(M_V8MIN) << 29;
        uint32_t raddrb;
        if (instr.ALU.srcB.tag == REG) {
          raddrb = 48;
        }
        else {
          uint32_t n = (uint32_t) instr.ALU.srcB.smallImm.val;
          assert(n >= 1 || n <= 15);
          raddrb = 48 + n;
        }
        uint32_t raddra = 39;
        *low = mulOp | (raddrb << 12) | (raddra << 18);
        return;
*/
      } else {
				if (instr.isMul()) {
        	ret_instr.alu.mul.op = v3d::encodeMulOp(instr.ALU.op);
				} else {
        	ret_instr.alu.add.op = v3d::encodeAddOp(instr.ALU.op);
				}

				v3d_qpu_mux muxa, muxb;
        uint8_t raddra, raddrb; // in vc4, these are the read addresses for regfile A or B
				                        // The meaning is different for v3d

				Reg aReg  = instr.ALU.srcA.reg;
				Reg bReg  = instr.ALU.srcB.reg;

        if (instr.ALU.srcA.tag == REG && instr.ALU.srcB.tag == REG) {
        	// Both operands are registers
          RegTag aTag  = aReg.tag;
          RegTag bTag  = bReg.tag;

					breakpoint
/*
          // If operands are the same register
          if (aTag != NONE && aTag == bTag &&
                instr.ALU.srcA.reg.regId == instr.ALU.srcB.reg.regId) {
            if (aFile == REG_A) {
              raddra = encodeSrcReg(instr.ALU.srcA.reg, REG_A, &muxa);
              muxb = muxa; raddrb = 39;
            }
            else {
              raddrb = encodeSrcReg(instr.ALU.srcA.reg, REG_B, &muxa);
              muxb = muxa; raddra = 39;
            }
          }
          else {
            // Operands are different registers
            assert(aFile == NONE || bFile == NONE || aFile != bFile);
            if (aFile == REG_A || bFile == REG_B) {
              raddra = encodeSrcReg(instr.ALU.srcA.reg, REG_A, &muxa);
              raddrb = encodeSrcReg(instr.ALU.srcB.reg, REG_B, &muxb);
            }
            else {
              raddrb = encodeSrcReg(instr.ALU.srcA.reg, REG_B, &muxa);
              raddra = encodeSrcReg(instr.ALU.srcB.reg, REG_A, &muxb);
            }
          }
*/
        }
        else if (instr.ALU.srcB.tag == IMM) {
          // Second operand is a small immediate
					assert(instr.ALU.srcA.tag == REG);

					breakpoint
					SmallImm imm(instr.ALU.srcB.smallImm.val);
					ret_instr.sig.small_imm = true;
					ret_instr.raddr_b = imm.to_raddr();

					// The muxa/b fields select between regfiles A and B for the src registers
					// So, irrelevant for v3d
          raddra = v3d::encodeSrcReg_old(instr.ALU.srcA.reg); // TODO fix for v3d
        }
        else if (instr.ALU.srcA.tag == IMM) {
          // First operand is a small immediate
					assert(instr.ALU.srcB.tag == REG);
					assert(false);
/*
          raddra = encodeSrcReg(instr.ALU.srcB.reg, REG_A, &muxb);
          raddrb = (uint32_t) instr.ALU.srcA.smallImm.val;
          muxa   = 7;
*/
        }
        else {
          assert(false);  // Both operands are small immediates; never expecting this
        }

/*
        *low = mulOp | addOp | (raddra << 18) | (raddrb << 12)
                     | (muxa << 9) | (muxb << 6)
                     | (muxa << 3) | muxb;
*/

				ret_instr.raddr_a = raddra;
				ret_instr.raddr_b = raddrb;

				// TODO: prob needs to be set for mul in some way
				ret_instr.alu.add.a = muxa;
				ret_instr.alu.add.b = muxb;
      }

			ret_instr.dump(true);
			ret << ret_instr;
    }
		break;

    // Halt
    case END: {
			ret << sync_tmu()
					<< end_program();
		}
		break;
    case TMU0_TO_ACC4: {
			ret << nop().ldtmu(r4);  // NOTE: added by WR
/*
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t raddra = 39 << 18;
      uint32_t raddrb = 39 << 12;
      uint32_t sig = instr.tag == END ? 0x30000000 : 0xa0000000;
      *high  = sig | waddr_add | waddr_mul;
      *low   = raddra | raddrb;
*/
    }
		break;

    // Semaphore increment/decrement
    case SINC:
    case SDEC: {
			assert(false);  // TODO examine
/*
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t sig = 0xe8000000;
      uint32_t incOrDec = (instr.tag == SINC ? 0 : 1) << 4;
      *high = sig | waddr_add | waddr_mul;
      *low = incOrDec | instr.semaId;
*/
    }
		break;

    // No-op & print instructions (ignored)
    case NO_OP:
			ret << nop();
			break;

    case PRI:
    case PRS:
    case PRF:
			breakpoint
			assert(false);  // TODO examine
/*
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      *high  = 0xe0000000 | waddr_add | waddr_mul;
      *low   = 0;
*/
		break;
		default:
  		fprintf(stderr, "v3d: missing case in encodeInstr\n");
		 	exit(EXIT_FAILURE);
  }


	for (auto &instr : ret) {
		cout << instr.mnemonic() << endl;
	}

	assert(!ret.empty());  // Something should really be returned back
	return ret;
}


void _encode(Seq<QPULib::Instr> &instrs, std::vector<uint64_t> &code) {
	OpCodes opcodes;

  for (int i = 0; i < instrs.numElems; i++) {
    QPULib::Instr instr = instrs.elems[i];
	
		auto result = v3d::encodeInstr(instr);
    opcodes.insert(opcodes.end(), result.begin(), result.end());
  }

	for (auto const &opcode : opcodes) {
		code << opcode.code();
	}
}


KernelDriver::KernelDriver() : QPULib::KernelDriver(V3dBuffer) {
}

KernelDriver::~KernelDriver() {
	delete qpuCodeMem;
	delete paramMem;
}


void KernelDriver::encode(int numQPUs, Seq<QPULib::Instr> &targetCode) {
	if (numQPUs != 1 && numQPUs != 8) {
		errors << "Num QPU's must be 1 or 8";
		return;
	}
	local_numQPUs = numQPUs;

	// Encode target instructions
	std::vector<uint64_t> code;
	_encode(targetCode, code);

	if (!local_errors.empty()) {
		breakpoint
	}

	errors << local_errors;
	local_errors.clear();


	// Allocate memory for the QPU code
	qpuCodeMem = new v3d::SharedArray<uint64_t>(code.size());
	qpuCodeMem->copyFrom(code);  // Copy kernel to code memory


	// Allocate memory for the parameters
	int numWords = (12*MAX_KERNEL_PARAMS + 12*2);
	paramMem = new v3d::SharedArray<uint32_t>;
	paramMem->alloc(numWords);

	qpuCodeMemOffset = 8*code.size();  // TODO check if correct
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t>* params) {
	v3d::invoke(numQPUs, *qpuCodeMem, qpuCodeMemOffset, params);
}

}  // namespace v3d
}  // namespace QPULib

#endif  // QPU_MODE
