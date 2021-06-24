#include "Target/Emulator.h"
#include <cmath>
#include "Support/basics.h"  // fatal()
#include "EmuSupport.h"
#include "Common/Queue.h"
#include "Common/SharedArray.h"
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
    case SPECIAL_SFU_RECIP:     value = v.recip();      break;
    case SPECIAL_SFU_RECIPSQRT: value = v.recip_sqrt(); break;
    case SPECIAL_SFU_EXP:       value = v.exp();        break;
    case SPECIAL_SFU_LOG:       value = v.log();        break;
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
        r4[i].floatVal = value[i].floatVal;
      }
      timer = -1;
    }
  }

private:
  Vec value;       // Last result of SFU unit call
  int timer = -1;  // Number of cycles to wait for SFU result.
};


// State of a single QPU.
struct QPUState {
  int id = 0;                          // QPU id
  int nextUniform = -2;                // Pointer to next uniform to read
  Seq<Vec> loadBuffer = 8;             // Load buffer for loads via TMU, 8 is initial size

  int readPitch = 0;                   // Read pitch
  int writeStride = 0;                 // Write stride

  bool running = false;                // Is QPU active, or has it halted?
  int pc = 0;                          // Program counter
  Vec* regFileA = nullptr;             // Register file A
  int sizeRegFileA = 0;                // (and size)
  Vec* regFileB = nullptr;             // Register file B
  int sizeRegFileB = 0;                // (and size)
  Vec accum[6];                        // Accumulator registers
  bool negFlags[NUM_LANES];            // Negative flags
  bool zeroFlags[NUM_LANES];           // Zero flags

  DMAAddr dmaLoad;                     // DMA load address
  DMAAddr dmaStore;                    // DMA store address
  DMALoadReq dmaLoadSetup;             // DMA load setup register
  DMAStoreReq dmaStoreSetup;           // DMA store setup register
  Queue<2, VPMLoadReq> vpmLoadQueue;   // VPM load queue
  VPMStoreReq vpmStoreSetup;           // VPM store setup

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
struct State : public EmuState {
  QPUState qpu[MAX_QPUS];  // State of each QPU
  Data emuHeap;

  State(int in_num_qpus, IntList const &in_uniforms) : EmuState(in_num_qpus, in_uniforms, true) {}
};


Vec DMA_readReg(QPUState* s, State* g, Reg reg, bool &handled) {
  assert(reg.tag == SPECIAL);

  Vec v(0);
  handled = true;

  switch (reg.regId) {
      case SPECIAL_VPM_READ: {
        // Make sure there's a VPM load request waiting
        assert(!s->vpmLoadQueue.isEmpty());
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
      }
      return v;

      case SPECIAL_DMA_LD_WAIT: {
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
     }
     return v; // Return value unspecified

      case SPECIAL_DMA_ST_WAIT: {
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

    default:
      handled = false;
      break;
  }

  return v;
}

}  // anon namespace


/**
 * Read a vector register
 */
Vec readReg(QPUState* s, State* g, Reg reg) {
  int r = reg.regId;
  Vec v(0);

  switch (reg.tag) {
    case NONE:
      return v;

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
        return EmuState::index_vec;
      } else if (reg.regId == SPECIAL_UNIFORM) {
        assertq(false, "readReg(): not expecting SPECIAL_UNIFORM to be handled any more");
      } else if (reg.regId == SPECIAL_QPU_NUM) {
        return Vec(s->id);
      } else {
        bool handled;
        v = DMA_readReg(s, g, reg, handled);
        if (handled) {
          return v; // Return value unspecified, don't care at this point
        }
      }
      fatal("V3DLib: can't read special register");
      break;

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


// ============================================================================
// Interpret a small immediate operand
// ============================================================================

Vec evalSmallImm(QPUState* s, EncodedSmallImm imm) {
  Vec v;
  Word w = decodeSmallLit(imm.val);

  for (int i = 0; i < NUM_LANES; i++) {
    v[i] = w;
  }

  return v;
}


Vec readRegOrImm(QPUState* s, State &state, RegOrImm const &src) {
  if (src.is_reg()) {
    return readReg(s, &state, src.reg());
  } else {
    return evalSmallImm(s, src.imm());
  }
}


// ============================================================================
// Emulator
// ============================================================================

/**
 * @param numQPUs   Number of QPUs active
 * @param instrs    Instruction sequence
 * @param maxReg    Max reg id used
 * @param uniforms  Kernel parameters
 * @param heap
 */
void emulate(int numQPUs, Instr::List &instrs, int maxReg, IntList &uniforms, BufferObject &heap) {
  State state(numQPUs, uniforms);
  state.emuHeap.heap_view(heap);

  // Initialise state
  for (int i = 0; i < numQPUs; i++) {
    QPUState &q = state.qpu[i];
    q.id                 = i;
    q.init(maxReg);
  }

  bool anyRunning = true;

  while (anyRunning) {
    auto ALWAYS = AssignCond::Tag::ALWAYS;
    anyRunning = false;

    // Execute an instruction in each active QPU
    for (int i = 0; i < numQPUs; i++) {
      QPUState* s = &state.qpu[i];

      if (s->running) {
        anyRunning = true;
        assert(s->pc < instrs.size());

        s->upkeep();

        //
        // Run next instruction
        //
        Instr const instr = instrs.get(s->pc++);

        if (instr.break_point()) {
#ifdef DEBUG
          printf("Emulator: hit breakpoint\n");
          breakpoint
#endif
        }

        switch (instr.tag) {
          case LI: {
            Vec imm(instr.LI.imm);
            writeReg(s, &state, instr.set_cond().flags_set(), instr.assign_cond(), instr.dest(), imm);
          }
          break;

          case ALU:
          if (!instr.ALU.op.isNOP()) {
            Vec a;
            Vec b;

            if (instr.isUniformLoad()) {
              a = state.get_uniform(s->id, s->nextUniform);
              b = a; 
            } else {
              a = readRegOrImm(s, state, instr.ALU.srcA);
              b = readRegOrImm(s, state, instr.ALU.srcB);
            }

            Vec result;
            result.apply(instr.ALU.op, a, b);

            writeReg(s, &state, instr.set_cond().flags_set(), instr.assign_cond(), instr.dest(), result);
          }
          break;

          case BR: {  // Branch to target
            if (checkBranchCond(s, instr.branch_cond())) {
              BranchTarget t = instr.branch_target();
              if (t.relative && !t.useRegOffset) {
                s->pc += 3+t.immOffset;
              } else {
                fatal("V3DLib: found unsupported form of branch target");
              }
            }
          }
          break;

          case RECV: {                             // receive load-via-TMU response
            assert(s->loadBuffer.size() > 0);
            Vec val = s->loadBuffer.remove(0);
            AssignCond always;
            always.tag = ALWAYS;
            writeReg(s, &state, false, always, instr.dest(), val);
          }
          break;

          case SINC: if (state.sema_inc(instr.semaId)) s->pc--; break;
          case SDEC: if (state.sema_dec(instr.semaId)) s->pc--; break;

          case END:                                // End program (halt)
            s->running = false;
            break;

          case BRL:                                // Branch to label
          case LAB:                                // Label
            fatal("V3DLib: emulator does not support labels");
            // Fall-thru
          case NO_OP:
          case IRQ:
          case INIT_BEGIN:
          case INIT_END:
            break;  // ignore

          default: assert(false);
        }
      }
    }
  }
}

}  // namespace V3DLib

