#include "Target/Emulator.h"
#include <cmath>
#include "Support/basics.h"  // fatal()
#include "EmuSupport.h"
#include "Common/Queue.h"
#include "Common/SharedArray.h"
#include "Target/instr/Instr.h"
#include "Target/SmallLiteral.h"
#include "BufferObject.h"

namespace V3DLib {

namespace {

class SFU {
public:

  /**
   * @return true if input handled, false otherwise
   */
  bool writeReg(Reg dest, Vec v) {
    if (dest.tag != SPECIAL) return false;
    bool handled = true;

    switch (dest.regId) {
    case SPECIAL_SFU_RECIP:
      for (int i = 0; i < NUM_LANES; i++) {
        float a = v[i].floatVal;
        value[i] = (a != 0)?1/a:0;      // TODO: not sure about value safeguard
      }
      break;
    case SPECIAL_SFU_RECIPSQRT:
      for (int i = 0; i < NUM_LANES; i++) {
        float a = (float) ::sqrt(v[i].floatVal);
        value[i] = (a != 0)?1/a:0;      // TODO: not sure about value safeguard
      }
      break;
    case SPECIAL_SFU_EXP:
      for (int i = 0; i < NUM_LANES; i++) {
        float a = v[i].floatVal;
        value[i] = (float) ::exp2(a);
      }
      break;
    case SPECIAL_SFU_LOG:
      for (int i = 0; i < NUM_LANES; i++) {
        float a = v[i].floatVal;
        value[i] = (float) ::log2(a);  // TODO what would lg2(0) return?
      }
      break;
    default:
      handled = false;
      break;
    }

    if (handled) {
      assertq(timer == -1, "SFU is running on SFU function call", true);
      timer = 3;
    }

    return handled;
  }


  void upkeep(Vec &r4) {
    if (timer > 0) {
      timer--;
    }

    if (timer == 0) {
      // finish pending SFU function
      for (int i = 0; i < NUM_LANES; i++) {
        r4[i].floatVal = value[i];
      }
      timer = -1;
    }
  }

private:
  float value[NUM_LANES];              // Last result of SFU unit call
  int timer = -1;                      // Number of cycles to wait for SFU result.
};


// State of a single QPU.
struct QPUState {
  int id = 0;                          // QPU id
  int numQPUs = 0;                     // QPU count
  bool running = false;                // Is QPU active, or has it halted?
  int pc = 0;                          // Program counter
  Vec* regFileA = nullptr;             // Register file A
  int sizeRegFileA = 0;                // (and size)
  Vec* regFileB = nullptr;             // Register file B
  int sizeRegFileB = 0;                // (and size)
  Vec accum[6];                        // Accumulator registers
  bool negFlags[NUM_LANES];            // Negative flags
  bool zeroFlags[NUM_LANES];           // Zero flags
  int nextUniform = -2;                // Pointer to next uniform to read
  DMAAddr dmaLoad;                     // DMA load address
  DMAAddr dmaStore;                    // DMA store address
  DMALoadReq dmaLoadSetup;             // DMA load setup register
  DMAStoreReq dmaStoreSetup;           // DMA store setup register
  Queue<2, VPMLoadReq> vpmLoadQueue;   // VPM load queue
  VPMStoreReq vpmStoreSetup;           // VPM store setup
  int readPitch = 0;                   // Read pitch
  int writeStride = 0;                 // Write stride
  SmallSeq<Vec> loadBuffer;            // Load buffer for loads via TMU

  SFU sfu;

  QPUState() {
    dmaLoad.active     = false;
    dmaStore.active    = false;
    for (int i = 0; i < NUM_LANES; i++) negFlags[i] = false;
    for (int i = 0; i < NUM_LANES; i++) zeroFlags[i] = false;
  }


  ~QPUState() {
    delete [] regFileA;
    delete [] regFileB;
  }

  void init(int maxReg) {
    running            = true;
    regFileA           = new Vec [maxReg+1];
    sizeRegFileA       = maxReg+1;
    regFileB           = new Vec [maxReg+1];
    sizeRegFileB       = maxReg+1;
  }

  void upkeep() {
    sfu.upkeep(accum[4]);
  }
};


/**
 * State of the VideoCore
 */
struct State {
  QPUState qpu[MAX_QPUS];  // State of each QPU
  Seq<int32_t> uniforms;   // Kernel parameters
  Word vpm[VPM_SIZE];      // Shared VPM memory
  Seq<char>* output;       // Output for print statements
  int sema[16];            // Semaphores
  SharedArray<uint32_t> emuHeap;

  State() {
    // Initialise semaphores
    for (int i = 0; i < 16; i++) sema[i] = 0;
  }
};

}


/**
 * Read a vector register
 */
Vec readReg(QPUState* s, State* g, Reg reg) {
  int r = reg.regId;
  Vec v;

  // Initialize v to prevent warnings on uninitialized errors
  for (int i = 0; i < NUM_LANES; i++)
    v[i].intVal = 0;

  switch (reg.tag) {
    case REG_A:
      assert(r >= 0 && r < s->sizeRegFileA);
      return s->regFileA[r];
    case REG_B:
      assert(r >= 0 && r < s->sizeRegFileB);
      return s->regFileB[reg.regId];
    case ACC:
      assert(r >= 0 && r <= 5);
      return s->accum[r];
    case SPECIAL:
      if (reg.regId == SPECIAL_ELEM_NUM) {
        for (int i = 0; i < NUM_LANES; i++)
          v[i].intVal = i;
        return v;
      } else if (reg.regId == SPECIAL_UNIFORM) {
        assert(s->nextUniform < g->uniforms.size());
        for (int i = 0; i < NUM_LANES; i++)
          if (s->nextUniform == -2)
            v[i].intVal = s->id;
          else if (s->nextUniform == -1)
            v[i].intVal = s->numQPUs;
          else
            v[i].intVal = g->uniforms[s->nextUniform];
        s->nextUniform++;
        return v;
      } else if (reg.regId == SPECIAL_QPU_NUM) {
        for (int i = 0; i < NUM_LANES; i++)
          v[i].intVal = s->id;
        return v;
      } else if (reg.regId == SPECIAL_VPM_READ) {
        // Make sure there's a VPM load request waiting
        assert(! s->vpmLoadQueue.isEmpty());
        VPMLoadReq* req = s->vpmLoadQueue.first();
        assert(req->numVecs > 0);
        if (req->hor) {
          // Horizontal load
          for (int i = 0; i < NUM_LANES; i++) {
            int index = (16*req->addr+i);
            assert(index < VPM_SIZE);
            v[i] = g->vpm[index];
          }
        } else {
          // Vertical load
          for (int i = 0; i < NUM_LANES; i++) {
            uint32_t x = req->addr & 0xf;
            uint32_t y = req->addr >> 4;
            int index = (y*16*16 + x + i*16);
            assert(index < VPM_SIZE);
            v[i] = g->vpm[index];
          }
        }
        req->numVecs--;
        req->addr = req->addr + req->stride;
        if (req->numVecs == 0) s->vpmLoadQueue.deq(); 
        return v;
      }
      else if (reg.regId == SPECIAL_DMA_LD_WAIT) {
        // Perform DMA load to completion
        if (s->dmaLoad.active == false) return v;
        DMALoadReq* req = &s->dmaLoadSetup;
        if (req->hor) {
          // Horizontal access
          uint32_t y = (req->vpmAddr >> 4) & 0x3f;
          for (int r = 0; r < req->numRows; r++) {
            uint32_t x = req->vpmAddr & 0xf;
            for (int i = 0; i < req->rowLen; i++) {
              uint32_t addr = (uint32_t) (s->dmaLoad.addr.intVal + (r * s->readPitch) + i*4);
              g->vpm[y*16 + x].intVal = g->emuHeap.phy(addr >> 2);
              x = (x+1) % 16;
            }
            y = (y+1) % 64;
          }
        } else {
          // Vertical access
          uint32_t x = req->vpmAddr & 0xf;
          for (int r = 0; r < req->numRows; r++) {
            uint32_t y = ((req->vpmAddr >> 4) + r*req->vpitch) & 0x3f;
            for (int i = 0; i < req->rowLen; i++) {
              uint32_t addr = (uint32_t) (s->dmaLoad.addr.intVal + (r * s->readPitch) + i*4);
              g->vpm[y*16 + x].intVal = g->emuHeap.phy(addr >> 2);
              y = (y+1) % 64;
            }
            x = (x+1) % 16;
          }
        }
        s->dmaLoad.active = false;
        return v; // Return value unspecified
      } else if (reg.regId == SPECIAL_DMA_ST_WAIT) {
        // Perform DMA store to completion
        if (s->dmaStore.active == false) return v;
        DMAStoreReq* req = &s->dmaStoreSetup;
        uint32_t memAddr = s->dmaStore.addr.intVal;

        if (req->hor) {
          // Horizontal access
          uint32_t y = (req->vpmAddr >> 4) & 0x3f;
          for (int r = 0; r < req->numRows; r++) {
            uint32_t x = req->vpmAddr & 0xf;
            for (int i = 0; i < req->rowLen; i++) {
              g->emuHeap.phy(memAddr >> 2) = g->vpm[y*16 + x].intVal;
              x = (x+1) % 16;
              memAddr = memAddr + 4;
            }
            y = (y+1) % 64;
            memAddr += s->writeStride;
          }
        } else {
          // Vertical access
          uint32_t x = req->vpmAddr & 0xf;
          for (int r = 0; r < req->numRows; r++) {
            uint32_t y = (req->vpmAddr >> 4) & 0x3f;
            for (int i = 0; i < req->rowLen; i++) {
              g->emuHeap.phy(memAddr >> 2) = g->vpm[y*16 + x].intVal;
              y = (y+1) % 64;
              memAddr = memAddr + 4;
            }
            x = (x+1) % 16;
            memAddr += s->writeStride;
          }
        }
        s->dmaStore.active = false;
        return v; // Return value unspecified
      }
      fatal("V3DLib: can't read special register");
    case NONE:
      return v;
    default:
      break;  // Should not happen
  }

  // Unreachable
  assert(false);
  return v;
}


// ============================================================================
// Check condition flags
// ============================================================================

// Given an assignment condition and an vector index, determine if the
// condition is true at that index using the implicit condition flags.

inline bool checkAssignCond(QPUState* s, AssignCond cond, int i) {
  using Tag = AssignCond::Tag;

  switch (cond.tag) {
    case Tag::NEVER:  return false;
    case Tag::ALWAYS: return true;
    case Tag::FLAG:
      switch (cond.flag) {
        case ZS: return  s->zeroFlags[i];
        case ZC: return !s->zeroFlags[i];
        case NS: return  s->negFlags[i];
        case NC: return !s->negFlags[i];
      }
  }

  // Unreachable
  assert(false);
  return false;
}


/**
 * Given a branch condition, determine if it evaluates to true using
 * the implicit condition flags.
 */
inline bool checkBranchCond(QPUState* s, BranchCond cond) {
  bool bools[NUM_LANES];
  switch (cond.tag) {
    case COND_ALWAYS: return true;
    case COND_NEVER: return false;
    case COND_ALL:
    case COND_ANY:
      for (int i = 0; i < NUM_LANES; i++)
        switch (cond.flag) {
          case ZS: bools[i] =  s->zeroFlags[i]; break;
          case ZC: bools[i] = !s->zeroFlags[i]; break;
          case NS: bools[i] =  s->negFlags[i];  break;
          case NC: bools[i] = !s->negFlags[i];  break;
          default: assert(false); break;
        }

      if (cond.tag == COND_ALL) {
        for (int i = 0; i < NUM_LANES; i++)
          if (! bools[i]) return false;
        return true;
      } else if (cond.tag == COND_ANY) {
        for (int i = 0; i < NUM_LANES; i++)
          if (bools[i]) return true;
        return false;
      }
    default:
      assertq(false, "checkBranchCond(): unexpected value");
      return false;
  }
}


/**
 * Write a vector to a register
 */
void writeReg(QPUState* s, State* g, bool setFlags, AssignCond cond, Reg dest, Vec v) {
  switch (dest.tag) {
    case REG_A:
    case REG_B:
    case ACC:
    case NONE:
      Vec* w;

      if (dest.tag == REG_A) {
        assert(dest.regId >= 0 && dest.regId < s->sizeRegFileA);
        w = &s->regFileA[dest.regId];
      } else if (dest.tag == REG_B) {
        assert(dest.regId >= 0 && dest.regId < s->sizeRegFileB);
        w = &s->regFileB[dest.regId];
      } else if (dest.tag == ACC) {
        assert(dest.regId >= 0 && dest.regId <= 5);
        w = &s->accum[dest.regId];
      }
      
      for (int i = 0; i < NUM_LANES; i++)
        if (checkAssignCond(s, cond, i)) {
          Word x = v[i];
          if (dest.tag != NONE) w->get(i) = x;

          if (setFlags) {
            s->zeroFlags[i] = x.intVal == 0;
            s->negFlags[i]  = x.intVal < 0;
          }
        }

      return;

    case SPECIAL:
      switch (dest.regId) {
        case SPECIAL_RD_SETUP: {
          int setup = v[0].intVal;
          if ((setup & 0xf0000000) == 0x90000000) {
            // Set read pitch
            int pitch = (setup & 0x1fff);
            s->readPitch = pitch;
            return;
          } else if ((setup & 0xc0000000) == 0) {
            // QPU only allows two VPM loads queued at a time
            assert(! s->vpmLoadQueue.isFull());
            // Create VPM load request
            VPMLoadReq req;
            req.numVecs = (setup >> 20) & 0xf;
            if (req.numVecs == 0) req.numVecs = 16;
            req.hor = ((setup >> 11) & 1);
            req.addr = setup & 0xff;
            req.stride = (setup >> 12) & 0x3f;
            if (req.stride == 0) req.stride = 64;
            // Add VPM load request to queue
            s->vpmLoadQueue.enq(req);
            return;
          } else if (setup & 0x80000000) {
            // DMA load setup
            DMALoadReq* req = &s->dmaLoadSetup;
            req->rowLen = (setup >> 20) & 0xf;
            if (req->rowLen == 0) req->rowLen = 16;
            req->numRows = (setup >> 16) & 0xf;
            if (req->numRows == 0) req->numRows = 16;
            req->vpitch = (setup >> 12) & 0xf;
            if (req->vpitch == 0) req->vpitch = 16;
            req->hor = (setup & 0x800) ? false : true;
            req->vpmAddr = (setup & 0x7ff);
            return;
          }
          break;
        }

        case SPECIAL_WR_SETUP: {
          int setup = v[0].intVal;
          if ((setup & 0xc0000000) == 0xc0000000) {
            // Set write stride
            int stride = setup & 0x1fff;
            s->writeStride = stride;
            return;
          } else if ((setup & 0xc0000000) == 0x80000000) {
            // DMA write setup
            DMAStoreReq* req = &s->dmaStoreSetup;
            req->rowLen = (setup >> 16) & 0x7f;
            if (req->rowLen == 0) req->rowLen = 128;
            req->numRows = (setup >> 23) & 0x7f;
            if (req->numRows == 0) req->numRows = 128;
            req->hor = (setup & 0x4000);
            req->vpmAddr = (setup >> 3) & 0x7ff;
            return;
          } else if ((setup & 0xc0000000) == 0) {
            VPMStoreReq req;
            req.hor = (setup >> 11) & 1;
            req.addr = setup & 0xff;
            req.stride = (setup >> 12) & 0x3f;
            if (req.stride == 0) req.stride = 64;
            s->vpmStoreSetup = req;
            return;
          }
          break;
        }

        case SPECIAL_VPM_WRITE: {
          VPMStoreReq* req = &s->vpmStoreSetup;
          if (req->hor) {
            // Horizontal store
            for (int i = 0; i < NUM_LANES; i++) {
              int index = (16*req->addr+i);
              assert(index < VPM_SIZE);
              g->vpm[index] = v[i];
            }
          } else {
            // Vertical store
            uint32_t x = req->addr & 0xf;
            uint32_t y = req->addr >> 4;
            for (int i = 0; i < NUM_LANES; i++) {
              int index = (y*16*16 + x + i*16);
              assert(index < VPM_SIZE);
              g->vpm[index] = v[i];
            }
          }
          req->addr = req->addr + req->stride;
          return;
        }

        case SPECIAL_DMA_LD_ADDR: {
          // Initiate DMA load
          assert(!s->dmaLoad.active);
          s->dmaLoad.active = true;
          s->dmaLoad.addr   = v[0];
          return;
        }

        case SPECIAL_DMA_ST_ADDR: {
          // Initiate DMA store
          assert(!s->dmaStore.active);
          s->dmaStore.active = true;
          s->dmaStore.addr   = v[0];
          return;
        }

        case SPECIAL_HOST_INT: {
          return;
        }

        case SPECIAL_TMU0_S: {
          assert(s->loadBuffer.size() < 4);
          Vec val;
          for (int i = 0; i < NUM_LANES; i++) {
            uint32_t a = (uint32_t) v[i].intVal;
            val[i].intVal = g->emuHeap.phy(a>>2);
          }
          s->loadBuffer.append(val);
          return;
        }

        default:
          if (s->sfu.writeReg(dest, v)) {
            return;
          }
          break;
      }

      assertq(false, "emulator: can not write to special register", true);
      return;

    default:
      assertq(false, "emulator: unexpected dest tag");
      break;
  }
}


/**
 * Interpret an immediate operand
 */
Vec evalImm(Imm imm) {
  Vec v;
  switch (imm.tag) {
    case IMM_INT32:
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = imm.intVal;
      return v;
    case IMM_FLOAT32:
      for (int i = 0; i < NUM_LANES; i++)
        v[i].floatVal = imm.floatVal;
      return v;
    case IMM_MASK:
      for (int i = 0; i < NUM_LANES; i++)
        v[i].intVal = (imm.intVal >> i) & 1;
      return v;
  }

  // Unreachable
  assert(false);
  return v;
}


// ============================================================================
// Interpret a small immediate operand
// ============================================================================

Vec evalSmallImm(QPUState* s, SmallImm imm) {
  Vec v;
  switch (imm.tag) {
    case SMALL_IMM: {
      Word w = decodeSmallLit(imm.val);
      for (int i = 0; i < NUM_LANES; i++)
        v[i] = w;
      return v;
    }

    case ROT_ACC:
    case ROT_IMM:
      // TODO How and when is this ever called???
      int amount = (imm.tag == ROT_IMM)? imm.val : (int) s->accum[4][0].intVal;
      return rotate(v, amount);
  }

  // Unreachable
  assert(false);
  return v;
}


Vec readRegOrImm(QPUState* s, State* g, RegOrImm src) {
  switch (src.tag) {
    case REG: return readReg(s, g, src.reg);
    case IMM: return evalSmallImm(s, src.smallImm);
  }

  // Unreachable
  assert(false);
  return Vec();
}


// ============================================================================
// ALU helpers
// ============================================================================

// Rotate right
inline int32_t rotRight(int32_t x, int32_t n) {
  uint32_t ux = (uint32_t) x;
  return (ux >> n) | (x << (32-n));
}


// Count leading zeros
inline int32_t clz(int32_t x) {
  int32_t count = 0;
  int32_t n = (int32_t) (sizeof(int)*8);
  for (int32_t i = 0; i < n; i++) {
    if (x & (1 << (n-1))) break;
    else count++;
    x <<= 1;
  }

  return count;
}


// ============================================================================
// ALU
// ============================================================================

Vec alu(QPUState* s, State* g, RegOrImm srcA, ALUOp op, RegOrImm srcB) {
  // First, obtain vector operands
  Vec a, b, c;
  a = readRegOrImm(s, g, srcA);

  if (srcA.tag == REG && srcB.tag == REG && srcA.reg == srcB.reg) {
    b = a;
  } else {
    b = readRegOrImm(s, g, srcB);
  }

  // Now evaluate the operation
  switch (op.value()) {
    case ALUOp::NOP:
      // No-op
      break;
    case ALUOp::A_FADD:
      // Floating-point add
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal + b[i].floatVal;
      break;
    case ALUOp::A_FSUB:
      // Floating-point subtract
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal - b[i].floatVal;
      break;
    case ALUOp::A_FMIN:
      // Floating-point min
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal < b[i].floatVal
                      ? a[i].floatVal : b[i].floatVal;
      break;
    case ALUOp::A_FMAX:
      // Floating-point max
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal > b[i].floatVal
                      ? a[i].floatVal : b[i].floatVal;
      break;
    case ALUOp::A_FMINABS:
      // Floating-point min of absolute values
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = fabs(a[i].floatVal) < fabs(b[i].floatVal)
                      ? a[i].floatVal : b[i].floatVal;
      break;
    case ALUOp::A_FMAXABS:
      // Floating-point max of absolute values
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = fabs(a[i].floatVal) > fabs(b[i].floatVal)
                      ? a[i].floatVal : b[i].floatVal;
      break;
    case ALUOp::A_FtoI:
      // Float to signed integer
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = (int) a[i].floatVal;
      break;
    case ALUOp::A_ItoF:
      // Signed integer to float
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = (float) a[i].intVal;
      break;
    case ALUOp::A_ADD:
      // Integer add
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal + b[i].intVal;
      break;
    case ALUOp::A_SUB:
      // Integer subtract
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal - b[i].intVal;
      break;
    case ALUOp::A_SHR:
      // Integer shift right
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = (int32_t) ((uint32_t) a[i].intVal >> b[i].intVal);
      break;
    case ALUOp::A_ASR:
      // Integer arithmetic shift right
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal >> b[i].intVal;
      break;
     case ALUOp::A_ROR:
      // Integer rotate right
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = rotRight(a[i].intVal, b[i].intVal);
      break;
     case ALUOp::A_SHL:
      // Integer shift left
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal << b[i].intVal;
      break;
    case ALUOp::A_MIN:
      // Integer min
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal < b[i].intVal
                    ? a[i].intVal : b[i].intVal;
      break;
    case ALUOp::A_MAX:
      // Integer max
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal > b[i].intVal
                    ? a[i].intVal : b[i].intVal;
      break;
    case ALUOp::A_BAND:
      // Bitwise and
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal & b[i].intVal;
      break;
    case ALUOp::A_BOR:
      // Bitwise or
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal | b[i].intVal;
      break;
    case ALUOp::A_BXOR:
      // Bitwise xor
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal ^ b[i].intVal;
      break;
    case ALUOp::A_BNOT:
      // Bitwise not
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = ~a[i].intVal;
      break;
    case ALUOp::A_CLZ:
      // Count leading zeros
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = clz(a[i].intVal);
      break;
    case ALUOp::M_FMUL:
      // Floating-point multiply
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal * b[i].floatVal;
      break;
    case ALUOp::M_MUL24:
      // Integer multiply (24-bit)
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = (a[i].intVal & 0xffffff) * (b[i].intVal & 0xffffff);
      break;
    case ALUOp::M_ROTATE:
      // Vector rotation
      c = rotate(a, (int) b[0].intVal);
      break;

    case ALUOp::A_V8ADDS:
    case ALUOp::A_V8SUBS:
    case ALUOp::M_V8MUL:
    case ALUOp::M_V8MIN:
    case ALUOp::M_V8MAX:
    case ALUOp::M_V8ADDS:
    case ALUOp::M_V8SUBS:
    default: {
      char buf[64];
      sprintf(buf, "V3DLib: unsupported operator %i", op.value());
      fatal(buf);
    }
  }

  return c;
}


// ============================================================================
// Emulator
// ============================================================================

void emulate(
  int numQPUs,
  Seq<Instr>* instrs,
  int maxReg,
  Seq<int32_t> &uniforms,
  BufferObject &heap,
  Seq<char>* output
) {
  State state;
  state.output = output;

  state.uniforms = uniforms;
  // Add final dummy uniform
  // See Note 1, function `invoke()` in `vc4/Invoke.cpp`.
  state.uniforms << 0;

  state.emuHeap.heap_view(heap);

  // Initialise state
  for (int i = 0; i < numQPUs; i++) {
    QPUState &q = state.qpu[i];

    q.id                 = i;
    q.numQPUs            = numQPUs;
    q.init(maxReg);
  }

  // Protection against locks due to semaphore waiting
  int const MAX_SEMAPHORE_WAIT = 1024;
  int semaphore_wait_count = 0;

  bool anyRunning = true;

  while (anyRunning) {
    auto ALWAYS = AssignCond::Tag::ALWAYS;
    anyRunning = false;

    // Execute an instruction in each active QPU
    for (int i = 0; i < numQPUs; i++) {
      QPUState* s = &state.qpu[i];

      if (s->running) {
        anyRunning = true;
        assert(s->pc < instrs->size());

        s->upkeep();

        //
        // Run next instruction
        //
        Instr const instr = instrs->get(s->pc++);
        switch (instr.tag) {
          // Load immediate
          case LI: {
            Vec imm = evalImm(instr.LI.imm);
            writeReg(s, &state, instr.setCond().flags_set(), instr.LI.cond, instr.LI.dest, imm);
            break;
          }
          // ALU operation
          case ALU: {
            Vec result = alu(s, &state, instr.ALU.srcA, instr.ALU.op, instr.ALU.srcB);
            if (!instr.ALU.op.isNOP())
              writeReg(s, &state, instr.setCond().flags_set(), instr.ALU.cond, instr.ALU.dest, result);
            break;
          }
          // End program (halt)
          case END: {
            s->running = false;
            break;
          }
          // Branch to target
          case BR: {
            if (checkBranchCond(s, instr.BR.cond)) {
              BranchTarget t = instr.BR.target;
              if (t.relative && !t.useRegOffset) {
                s->pc += 3+t.immOffset;
              } else {
                fatal("V3DLib: found unsupported form of branch target");
              }
            }
            break;
          }

          case BRL:                                // Branch to label
          case LAB:                                // Label
            fatal("V3DLib: emulator does not support labels");
          case NO_OP:
            break;
          case PRS: {                              // print string
            emitStr(state.output, instr.PRS);
            break;
          }
          case PRI: {                              // print integer
            Vec x = readReg(s, &state, instr.PRI);
            printIntVec(state.output, x);
            break;
          }
          case PRF: {                              // print float
            Vec x = readReg(s, &state, instr.PRF);
            printFloatVec(state.output, x);
            break;
          }
          case RECV: {                             // receive load-via-TMU response
            assert(s->loadBuffer.size() > 0);
            Vec val = s->loadBuffer.remove(0);
            AssignCond always;
            always.tag = ALWAYS;
            writeReg(s, &state, false, always, instr.RECV.dest, val);
            break;
          }
          case TMU0_TO_ACC4: {                     // Read from TMU0 into accumulator 4
            assert(s->loadBuffer.size() > 0);
            Vec val = s->loadBuffer.remove(0);
            AssignCond always;
            always.tag = ALWAYS;
            Reg dest;
            dest.tag = ACC;
            dest.regId = 4;
            writeReg(s, &state, false, always, dest, val);
            break;
          }
          case IRQ:                                 // Host IRQ
            break;
          case SINC: {                              // Semaphore increment
            assert(instr.semaId >= 0 && instr.semaId <= 15);
            if (state.sema[instr.semaId] == 15) {
              semaphore_wait_count++;
              assertq(semaphore_wait_count < MAX_SEMAPHORE_WAIT, "Semaphore wait for SINC appears to be stuck");
              s->pc--;
            } else {
              semaphore_wait_count = 0;
              state.sema[instr.semaId]++;
            }
            break;
          }
          case SDEC: {                              // Semaphore decrement
            assert(instr.semaId >= 0 && instr.semaId <= 15);
            if (state.sema[instr.semaId] == 0) {
              semaphore_wait_count++;
              assertq(semaphore_wait_count < MAX_SEMAPHORE_WAIT, "Semaphore wait for SDEC appears to be stuck");
              s->pc--;
            } else {
              semaphore_wait_count = 0;
              state.sema[instr.semaId]--;
            }
            break;
          }

          case INIT_BEGIN:
          case INIT_END:
            break;  // ignore

          // Should not be reached
          default: assert(false);
        }
      }
    }
  }
}

}  // namespace V3DLib

