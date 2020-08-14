#include "KernelDrivers.h"
#include <vector>
#include "Target/Encode.h"
#include "VideoCore/VideoCore.h"
#include "VideoCore/Invoke.h"
#include "Source/Stmt.h"
#include "v3d/Invoke.h"
#include "v3d/Instr.h"
#include "debug.h"


namespace QPULib {

namespace vc4 {
using namespace VideoCore;

KernelDriver::KernelDriver() : QPULib::KernelDriver(Vc4Buffer) {
	enableQPUs();
}


KernelDriver::~KernelDriver() {
	disableQPUs();
	delete qpuCodeMem;
}


void KernelDriver::kernelFinish() {
	// QPU code to cleanly exit
	QPULib::kernelFinish();
}


void KernelDriver::encode(Seq<Instr> &targetCode) {
    // Encode target instrs into array of 32-bit ints
    Seq<uint32_t> code;
    QPULib::encode(&targetCode, &code);

    // Allocate memory for QPU code and parameters
    int numWords = code.numElems + 12*MAX_KERNEL_PARAMS + 12*2;

		qpuCodeMem = new VideoCore::SharedArray<uint32_t>;
    qpuCodeMem->alloc(numWords);

    // Copy kernel to code memory
    int offset = 0;
    for (int i = 0; i < code.numElems; i++) {
      (*qpuCodeMem)[offset++] = code.elems[i];
    }
    qpuCodeMemOffset = offset;
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t>* params) {
	assert(qpuCodeMem != nullptr);
	QPULib::invoke(numQPUs, *qpuCodeMem, qpuCodeMemOffset, params);
}

}  // namespace vc4


namespace v3d {
using namespace QPULib::v3d::instr;

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


void encodeDestReg(QPULib::Instr const &src_instr, QPULib::v3d::instr::Instr &dst_instr) {
	Reg reg = src_instr.ALU.dest;

	auto set_dest = [&src_instr, &dst_instr] (Location const &loc) {
		if (src_instr.isMul()) {
			breakpoint

			dst_instr.alu.mul.waddr = loc.to_waddr();
			dst_instr.alu.mul.output_pack = loc.output_pack();
		} else {
			dst_instr.alu.add.waddr = loc.to_waddr();
			dst_instr.alu.add.output_pack = loc.output_pack();
		}
	};


  switch (reg.tag) {
		// NOTE: v3d can access 64 registers in regfile
		// TODO: see how we can change this
    case REG_A:
    case REG_B:
      assert(reg.regId >= 0 && reg.regId < 32);
			if (reg.regId != 0) {
				breakpoint
			}
			set_dest(RFAddress((uint8_t) reg.regId));
      return;
    case ACC:
			breakpoint
      assert(reg.regId >= 0 && reg.regId <= 5);
			switch(reg.regId) {
				case 0: set_dest(r0); break;
				case 1: set_dest(r1); break;
				case 2: set_dest(r2); break;
				case 3: set_dest(r3); break;
				case 4: set_dest(r4); break;
				case 5: set_dest(r5); break;
			}
      return;
    case SPECIAL:
			assert(false);
/*
      switch (reg.regId) {
        case SPECIAL_RD_SETUP:    return 49;
        case SPECIAL_WR_SETUP:    return 49;
        case SPECIAL_DMA_LD_ADDR: return 50;
        case SPECIAL_DMA_ST_ADDR: return 50;
        case SPECIAL_VPM_WRITE:   return 48;
        case SPECIAL_HOST_INT:    return 38;
        case SPECIAL_TMU0_S:      return 56;
        default:                  break;
      }
*/
    case NONE: // nop - ignore, dst_instr already init'ed like that
			return;
  }

  fprintf(stderr, "QPULib: missing case in encodeDestReg\n");
	assert(false);
}


uint64_t encodeInstr(QPULib::Instr instr) {
	uint64_t ret = 0;

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
			assert(false);  // TODO examine
/*
      RegTag file;
      uint32_t cond = encodeAssignCond(instr.LI.cond) << 17;
      uint32_t waddr_add = encodeDestReg(instr.LI.dest, &file) << 6;
      uint32_t waddr_mul = 39;
      uint32_t ws   = (file == REG_A ? 0 : 1) << 12;
      uint32_t sf   = (instr.LI.setFlags ? 1 : 0) << 13;
      *high         = 0xe0000000 | cond | ws | sf | waddr_add | waddr_mul;
      *low          = (uint32_t) instr.LI.imm.intVal;
*/
      return ret;
    }

    // Branch
    case BR: {
			assert(false);  // TODO examine
/*
      // Register offset not yet supported
      assert(! instr.BR.target.useRegOffset);

      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t cond = encodeBranchCond(instr.BR.cond) << 20;
      uint32_t rel  = (instr.BR.target.relative ? 1 : 0) << 19;
      *high = 0xf0000000 | cond | rel | waddr_add | waddr_mul;
      *low  = (uint32_t) 8*instr.BR.target.immOffset;
*/
      return ret;
    }

    // ALU
    case ALU: {

/*
	from gdb

	instr = {
		tag = QPULib::ALU,
		{
			LI = {
				setFlags = false,
				cond = {
					tag = QPULib::ALWAYS, flag = QPULib::ZS
				},
				dest = {
					tag = QPULib::REG_A, regId = 0
				},
				imm = {
					tag = QPULib::IMM_INT32,
					{
						intVal = 3, floatVal = 4.20389539e-45}
					}
			},
			ALU = {
      	setFlags = false,
				cond = {
					tag = QPULib::ALWAYS,
					flag = QPULib::ZS
				},
				dest = {
					tag = QPULib::REG_A,
					regId = 0
				},
				srcA = {
					tag = QPULib::REG,
					{
						reg = {
							tag = QPULib::SPECIAL,
							regId = 0
						},
						smallImm = {
            	tag = (QPULib::ROT_ACC | QPULib::ROT_IMM), val = 0
						}
					}
				},
				op = QPULib::A_BOR,
				srcB = {
					tag = QPULib::REG,
					{
						reg = {
							tag = QPULib::SPECIAL,
							regId = 0
						},
						smallImm = {
							tag = (QPULib::ROT_ACC | QPULib::ROT_IMM), val = 0
						}
					}
				}
			},
			BR = {
      	cond = {
					tag = QPULib::COND_ALL,
					flag = QPULib::ZC
				}, target = {
					relative = false,
					useRegOffset = false,
					regOffset = 0,
					immOffset = 0
				}
			},
			BRL = {
				cond = {
					tag = QPULib::COND_ALL,
					flag = QPULib::ZC
				},
				label = 0
			},
			label = 0,
			semaId = 0, 
    	RECV = {
				dest = {
					tag = QPULib::REG_A,
					regId = 1
				}
			},
			PRS = 0x0,
			PRI = {
				tag = QPULib::REG_A,
				regId = 1
			},
			PRF = {
				tag = QPULib::REG_A,
				regId = 1
			}
		}
	}
*/

// Register codes:
//
//  39 - ALU NOP

			breakpoint  // TODO examine
			QPULib::v3d::instr::Instr ret_instr;  // Note inits to nop-nop
      v3d::encodeDestReg(instr, ret_instr);
/*
      uint32_t sig   = ((instr.hasImm() || instr.isRot) ? 13 : 1) << 28;
      uint32_t cond  = encodeAssignCond(instr.ALU.cond) << (instr.isMul() ? 14 : 17);
      uint32_t ws;  // bitfield that selects between regfile A and B
			              // There is no such distinction on v3d, there is one regfile
      uint32_t sf    = (instr.ALU.setFlags ? 1 : 0) << 13;
      *high          = sig | cond | ws | sf | waddr_add | waddr_mul;
*/
      if (instr.ALU.op == M_ROTATE) {
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
      }
      else {
				if (instr.isMul()) {
        	ret_instr.alu.mul.op = v3d::encodeMulOp(instr.ALU.op);
				} else {
        	ret_instr.alu.add.op = v3d::encodeAddOp(instr.ALU.op);
				}

				v3d_qpu_mux muxa, muxb;
        uint8_t raddra, raddrb; // in vc4, these are the read addresses for regfile A or B
				                        // The meaning is different for v3d

        // Both operands are registers
        if (instr.ALU.srcA.tag == REG && instr.ALU.srcB.tag == REG) {
					breakpoint

          RegTag aTag  = instr.ALU.srcA.reg.tag;
          RegTag bTag  = instr.ALU.srcB.reg.tag;

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
					assert(false);
/*
          // Second operand is a small immediate
          raddra = encodeSrcReg(instr.ALU.srcA.reg, REG_A, &muxa);
          raddrb = (uint32_t) instr.ALU.srcB.smallImm.val;
          muxb   = 7;
*/
        }
        else if (instr.ALU.srcA.tag == IMM) {
					assert(false);
/*
          // First operand is a small immediate
          raddra = encodeSrcReg(instr.ALU.srcB.reg, REG_A, &muxb);
          raddrb = (uint32_t) instr.ALU.srcA.smallImm.val;
          muxa   = 7;
*/
        }
        else {
          // Both operands are small immediates
          assert(false);
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
			return ret_instr.code();  // NOTE: added by WR
    }

    // Halt
    case END:
    case TMU0_TO_ACC4: {
			assert(false);  // TODO examine
/*
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t raddra = 39 << 18;
      uint32_t raddrb = 39 << 12;
      uint32_t sig = instr.tag == END ? 0x30000000 : 0xa0000000;
      *high  = sig | waddr_add | waddr_mul;
      *low   = raddra | raddrb;
*/
      return ret;
    }

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
      return ret;
    }

    // No-op & print instructions (ignored)
    case NO_OP:
    case PRI:
    case PRS:
    case PRF:
			assert(false);  // TODO examine
/*
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      *high  = 0xe0000000 | waddr_add | waddr_mul;
      *low   = 0;
*/
      return ret;
  }

  fprintf(stderr, "v3d: missing case in encodeInstr\n");
 	exit(EXIT_FAILURE);
	return ret;
}


void _encode(Seq<QPULib::Instr> &instrs, std::vector<uint64_t> &code) {
  for (int i = 0; i < instrs.numElems; i++) {
    QPULib::Instr instr = instrs.elems[i];
    uint64_t opcode = v3d::encodeInstr(instr);
		code.push_back(opcode);
  }
}


KernelDriver::KernelDriver() : QPULib::KernelDriver(V3dBuffer) {
}

KernelDriver::~KernelDriver() {
	delete qpuCodeMem;
	delete paramMem;
}


void KernelDriver::encode(Seq<QPULib::Instr> &targetCode) {

	// Encode target instructions
	std::vector<uint64_t> code;
	_encode(targetCode, code);


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


