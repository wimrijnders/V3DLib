#include "KernelDrivers.h"
#include <vector>
#include "Target/Encode.h"
#include "VideoCore/VideoCore.h"
#include "VideoCore/Invoke.h"
#include "Source/Stmt.h"
#include "v3d/Invoke.h"
#include "debug.h"


namespace QPULib {

namespace vc4 {
using namespace VideoCore;

KernelDriver::KernelDriver() {
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

		qpuCodeMem = new SharedArray<uint32_t>;
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

namespace {

uint64_t encodeInstr(Instr instr) {
	uint64_t ret = 0;

	breakpoint

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

  // Encode core instrcution
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
			assert(false);  // TODO examine
/*
      RegTag file;
      bool isMul     = isMulOp(instr.ALU.op);
      bool hasImm    = instr.ALU.srcA.tag == IMM || instr.ALU.srcB.tag == IMM;
      bool isRot     = instr.ALU.op == M_ROTATE;
      uint32_t sig   = ((hasImm || isRot) ? 13 : 1) << 28;
      uint32_t cond  = encodeAssignCond(instr.ALU.cond) << (isMul ? 14 : 17);
      uint32_t dest  = encodeDestReg(instr.ALU.dest, &file);
      uint32_t waddr_add, waddr_mul, ws;
      if (isMul) {
        waddr_add = 39 << 6;
        waddr_mul = dest;
        ws        = (file == REG_B ? 0 : 1) << 12;
      }
      else {
        waddr_add = dest << 6;
        waddr_mul = 39;
        ws        = (file == REG_A ? 0 : 1) << 12;
      }
      uint32_t sf    = (instr.ALU.setFlags ? 1 : 0) << 13;
      *high          = sig | cond | ws | sf | waddr_add | waddr_mul;

      if (instr.ALU.op == M_ROTATE) {
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
      }
      else {
        uint32_t mulOp = (isMul ? encodeMulOp(instr.ALU.op) : 0) << 29;
        uint32_t addOp = (isMul ? 0 : encodeAddOp(instr.ALU.op)) << 24;

        uint32_t muxa, muxb;
        uint32_t raddra, raddrb;

        // Both operands are registers
        if (instr.ALU.srcA.tag == REG && instr.ALU.srcB.tag == REG) {
          RegTag aFile = regFileOf(instr.ALU.srcA.reg);
          RegTag bFile = regFileOf(instr.ALU.srcB.reg);
          RegTag aTag  = instr.ALU.srcA.reg.tag;
          RegTag bTag  = instr.ALU.srcB.reg.tag;

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
        }
        else if (instr.ALU.srcB.tag == IMM) {
          // Second operand is a small immediate
          raddra = encodeSrcReg(instr.ALU.srcA.reg, REG_A, &muxa);
          raddrb = (uint32_t) instr.ALU.srcB.smallImm.val;
          muxb   = 7;
        }
        else if (instr.ALU.srcA.tag == IMM) {
          // First operand is a small immediate
          raddra = encodeSrcReg(instr.ALU.srcB.reg, REG_A, &muxb);
          raddrb = (uint32_t) instr.ALU.srcA.smallImm.val;
          muxa   = 7;
        }
        else {
          // Both operands are small immediates
          assert(false);
        }
        *low = mulOp | addOp | (raddra << 18) | (raddrb << 12)
                     | (muxa << 9) | (muxb << 6)
                     | (muxa << 3) | muxb;
        return;
      }
*/
			return ret;  // NOTE: added by WR
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


void _encode(Seq<Instr> &instrs, std::vector<uint64_t> &code) {
  for (int i = 0; i < instrs.numElems; i++) {
    Instr instr = instrs.elems[i];
    uint64_t opcode = encodeInstr(instr);
		code.push_back(opcode);
  }
}


}  // anon namespace


KernelDriver::KernelDriver() {
}

KernelDriver::~KernelDriver() {
	delete qpuCodeMem;
}


void KernelDriver::encode(Seq<Instr> &targetCode) {

	// Encode target instructions
	std::vector<uint64_t> code;
	_encode(targetCode, code);

	// Allocate memory for QPU code and parameters
	int numBytes = 8*code.size() + 4*(12*MAX_KERNEL_PARAMS + 12*2);

	assert(false);  // TODO
	// qpuCodeMem = heap.alloc_view<uint64_t>(numBytes);

	// Copy kernel to code memory
	qpuCodeMem->copyFrom(code);

	qpuCodeMemOffset = 8*code.size();  // TODO check if correct
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t>* params) {
	v3d::invoke(numQPUs, *qpuCodeMem, qpuCodeMemOffset, params);
}

}  // namespace v3d
}  // namespace QPULib


