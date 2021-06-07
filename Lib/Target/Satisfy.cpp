#include "Satisfy.h"
#include <assert.h>
#include <stdio.h>
#include "Support/Platform.h"
#include "Liveness/Liveness.h"
#include "Target/instr/Mnemonics.h"
#include "Liveness/UseDef.h"

namespace V3DLib {
namespace {

/**
 * Remap register A to accumulator
 *
 * Return an instruction to move the contents of a register to an
 * accumulator, and change the use of that register in the given
 * instruction to the given accumulator.
 */
Instr remapAToAccum(Instr* instr, RegId acc) {
  assert(instr->ALU.srcA.is_reg());

  Reg src = instr->ALU.srcA.reg();
  instr->ALU.srcA.reg().tag    = ACC;
  instr->ALU.srcA.reg().regId  = acc;

  return Target::instr::mov(Reg(ACC, acc), src);
}


/**
 * Remap register B to accumulator
 */
Instr remapBToAccum(Instr* instr, RegId acc) {
  assert(instr->ALU.srcB.is_reg());

  Reg src = instr->ALU.srcB.reg();
  instr->ALU.srcB.reg().tag   = ACC;
  instr->ALU.srcB.reg().regId = acc;

  return Target::instr::mov(Reg(ACC, acc), src);
}


/**
 * When an instruction uses two (different) registers that are mapped
 * to the same register file, then remap one of them to an accumulator.
 */
bool resolveRegFileConflict(Instr* instr, Instr* newInstr) {
  if (instr->tag == ALU && instr->ALU.srcA.is_reg() && instr->ALU.srcB.is_reg()) {
    int rfa = regFileOf(instr->ALU.srcA.reg());
    int rfb = regFileOf(instr->ALU.srcB.reg());

    if (rfa != NONE && rfb != NONE) {
      bool conflict = rfa == rfb && !(instr->ALU.srcA.reg() == instr->ALU.srcB.reg());

      if (conflict) {
        *newInstr = remapAToAccum(instr, 0);
        return true;
      }
    }
  }
  return false;
}


/**
 * First pass for satisfy constraints: insert move-to-accumulator instructions
 */
Instr::List insertMoves_vc4(Instr::List &instrs) {
  if (!Platform::compiling_for_vc4())  {                           // Not an issue for v3d
    return instrs;
  }

  Instr::List newInstrs(instrs.size() * 2);

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    if (instr.tag == ALU && instr.ALU.srcA.is_imm() &&
        instr.ALU.srcB.is_reg() &&
        regFileOf(instr.ALU.srcB.reg()) == REG_B) {
      // Insert moves for an operation with a small immediate whose
      // register operand must reside in reg file B.
      newInstrs << remapBToAccum(&instr, 0);
    } else if (instr.tag == ALU && instr.ALU.srcB.is_imm() &&
             instr.ALU.srcA.is_reg() &&
             regFileOf(instr.ALU.srcA.reg()) == REG_B) {
      // Insert moves for an operation with a small immediate whose
      // register operand must reside in reg file B.
      newInstrs << remapAToAccum(&instr, 0);
    } else {
      // Insert moves for operands that are mapped to the same reg file
      Instr move;
      if (resolveRegFileConflict(&instr, &move)) {
        newInstrs << move;
      }
    }
    
    // Put current instruction into the new sequence
    newInstrs << instr;
  }

  return newInstrs;
}


Instr::List insertMoves(Instr::List &instrs) {
  Instr::List newInstrs(instrs.size() * 2);

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    if (instr.tag == ALU && instr.ALU.op.value() == ALUOp::M_ROTATE) {
      // Insert moves for horizontal rotate operations
      newInstrs << remapAToAccum(&instr, 0);

      if (instr.ALU.srcB.is_reg())
        newInstrs << remapBToAccum(&instr, 5);

      newInstrs << Instr::nop();
    }
    
    // Put current instruction into the new sequence
    newInstrs << instr;
  }

  return newInstrs;
}


/**
 * Second pass satisfy constraints: insert NOPs
 */
Instr::List insertNops(Instr::List &instrs) {
  Instr::List newInstrs(instrs.size() * 2);


  Instr prev = Instr::nop();

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    // 
    // For vc4, if an rf-register is set, you must wait one cycle before the value is available.
    //
    // If an rf-register is set, and used immediately in the next instruction, insert a NOP in between.
    //
    // v3d does not have this restriction.
    //
    if (Platform::compiling_for_vc4()) {
      Reg dst = prev.dst_reg();
      if (dst.tag != NONE) {
        bool needNop = dst.tag == REG_A || dst.tag == REG_B;  // rf-registers only

        if (needNop && instr.is_src_reg(dst)) {
          newInstrs << Instr::nop();
        }
      }
    }

    newInstrs << instr;                                           // Put current instruction into the new sequence


    // 
    // Insert NOPs in branch delay slots
    //
    if (instr.tag == BRL || instr.tag == END) {
      for (int j = 0; j < 3; j++)
        newInstrs << Instr::nop();

      prev = Instr::nop();
    }

    if (instr.tag != LAB) prev = instr;                           // Update previous instruction
  }

  return newInstrs;
}


/**
 * Return true for any instruction that doesn't read from the VPM
 */
bool notVPMGet(Instr instr) {
  for (auto const &useReg : instr.src_regs()) {
    if (useReg.tag == SPECIAL && useReg.regId == SPECIAL_VPM_READ)
      return false;
  }
  return true;
}


/**
 * Insert NOPs between VPM setup and VPM read, if needed
 */
Instr::List removeVPMStall(Instr::List &instrs) {
  Instr::List newInstrs(instrs.size() * 2);

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];
    if (instr.tag != VPM_STALL)
      newInstrs << instr;
    else {
      int numNops = 3;  // Number of nops to insert
      for (int j = 1; j <= 3; j++) {
        if ((i+j) >= instrs.size()) break;
        Instr next = instrs[i+j];
        if (next.tag == LAB) break;
        if (notVPMGet(next)) numNops--; else break;
      }

      for (int j = 0; j < numNops; j++)
        newInstrs << Instr::nop();
    }
  }

  return newInstrs;
}

}  // anon namespace


/**
 * Determine reg file of given register
 */
RegTag regFileOf(Reg r) {
  assert(r.tag <= SPECIAL);

  if (r.tag == REG_A) return REG_A;
  if (r.tag == REG_B) return REG_B;

  if (r.tag == SPECIAL) {
    switch(r.regId) {
    case SPECIAL_ELEM_NUM:
    case SPECIAL_RD_SETUP:
    case SPECIAL_DMA_LD_WAIT:
    case SPECIAL_DMA_LD_ADDR:
      return REG_A;
    case SPECIAL_QPU_NUM:
    case SPECIAL_WR_SETUP:
    case SPECIAL_DMA_ST_WAIT:
    case SPECIAL_DMA_ST_ADDR:
      return REG_B;
    default:
      break;
    }
  }

  return NONE;
}


/**
 * Satisfy VideoCore constraints
 *
 * Transform an instruction sequence to satisfy various VideoCore
 * constraints, including:
 *
 *   1. fill branch delay slots with NOPs;
 *
 *   2. introduce accumulators for operands mapped to the same
 *      register file;
 *
 *   3. introduce accumulators for horizontal rotation operands;
 *
 *   4. insert NOPs to account for data hazards: a destination
 *      register (assuming it's not an accumulator) cannot be read by
 *      the next instruction.
 */
void satisfy(Instr::List &instrs) {
  // Apply passes
  Instr::List newInstrs = insertMoves(instrs);
  newInstrs = insertMoves_vc4(newInstrs);
  newInstrs = insertNops(newInstrs);
  instrs = removeVPMStall(newInstrs);
}

}  // namespace V3DLib
