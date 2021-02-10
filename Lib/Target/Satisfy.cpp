#include "Satisfy.h"
#include <assert.h>
#include <stdio.h>
#include "Support/Platform.h"
#include "Target/Liveness.h"
#include "Target/instr/Instructions.h"

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
  assert(instr->ALU.srcA.tag == REG);

  Reg src = instr->ALU.srcA.reg;
  instr->ALU.srcA.reg.tag    = ACC;
  instr->ALU.srcA.reg.regId  = acc;

  return Target::instr::mov(Reg(ACC, acc), src);
}


/**
 * Remap register B to accumulator
 */
Instr remapBToAccum(Instr* instr, RegId acc) {
  assert(instr->ALU.srcB.tag == REG);

  Reg src = instr->ALU.srcB.reg;
  instr->ALU.srcB.reg.tag   = ACC;
  instr->ALU.srcB.reg.regId = acc;

  return Target::instr::mov(Reg(ACC, acc), src);
}


/**
 * When an instruction uses two (different) registers that are mapped
 * to the same register file, then remap one of them to an accumulator.
 */
bool resolveRegFileConflict(Instr* instr, Instr* newInstr) {
  if (instr->tag == ALU && instr->ALU.srcA.tag == REG && instr->ALU.srcB.tag == REG) {
    int rfa = regFileOf(instr->ALU.srcA.reg);
    int rfb = regFileOf(instr->ALU.srcB.reg);

    if (rfa != NONE && rfb != NONE) {
      bool conflict = rfa == rfb && !(instr->ALU.srcA.reg == instr->ALU.srcB.reg);

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
void insertMoves(Seq<Instr> &instrs, Seq<Instr>* newInstrs) {
  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    if (instr.tag == ALU && instr.ALU.op.value() == ALUOp::M_ROTATE) {
      // Insert moves for horizontal rotate operations
      *newInstrs << remapAToAccum(&instr, 0);

      if (instr.ALU.srcB.tag == REG)
        *newInstrs << remapBToAccum(&instr, 5);

      *newInstrs << Instr::nop();
    } else if (instr.tag == ALU && instr.ALU.srcA.tag == IMM &&
             instr.ALU.srcB.tag == REG &&
             regFileOf(instr.ALU.srcB.reg) == REG_B) {
      // Insert moves for an operation with a small immediate whose
      // register operand must reside in reg file B.
      *newInstrs << remapBToAccum(&instr, 0);
    } else if (instr.tag == ALU && instr.ALU.srcB.tag == IMM &&
             instr.ALU.srcA.tag == REG &&
             regFileOf(instr.ALU.srcA.reg) == REG_B) {
      // Insert moves for an operation with a small immediate whose
      // register operand must reside in reg file B.
      newInstrs->append(remapAToAccum(&instr, 0));
    } else if(Platform::compiling_for_vc4())  {                           // Not an issue for v3d
      // Insert moves for operands that are mapped to the same reg file
      Instr move;
      if (resolveRegFileConflict(&instr, &move))
        newInstrs->append(move);
    }
    
    // Put current instruction into the new sequence
    newInstrs->append(instr);
  }
}


/**
 * Second pass satisfy constraints: insert NOPs
 */
void insertNops(Seq<Instr> &instrs, Seq<Instr> &newInstrs) {
  // Use/def sets
  UseDefReg mySet, prevSet;

  // Previous instruction
  Instr prev = Instr::nop();

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    // Insert NOPs to avoid data hazards
    useDefReg(prev, &prevSet);
    useDefReg(instr, &mySet);
    for (int j = 0; j < prevSet.def.size(); j++) {
      Reg defReg = prevSet.def[j];
      bool needNop = defReg.tag == REG_A || defReg.tag == REG_B;

      if (needNop && mySet.use.member(defReg)) {
        newInstrs << Instr::nop();
        break;
      }
    }

    // Put current instruction into the new sequence
    newInstrs << instr;

    // Insert NOPs in branch delay slots
    if (instr.tag == BRL || instr.tag == END) {
      for (int j = 0; j < 3; j++)
        newInstrs << Instr::nop();

      prev = Instr::nop();
    }

    // Update previous instruction
    if (instr.tag != LAB) prev = instr;
  }
}


/**
 * Return true for any instruction that doesn't read from the VPM
 */
bool notVPMGet(Instr instr) {
  // Use/def sets
  UseDefReg useDef;

  useDefReg(instr, &useDef);
  for (int i = 0; i < useDef.use.size(); i++) {
    Reg useReg = useDef.use[i];
    if (useReg.tag == SPECIAL && useReg.regId == SPECIAL_VPM_READ)
      return false;
  }
  return true;
}


/**
 * Insert NOPs between VPM setup and VPM read, if needed
 */
void removeVPMStall(Seq<Instr> &instrs, Seq<Instr> &newInstrs) {
  // Use/def sets
  UseDefReg useDef;

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
}

}  // anon namespace


// ==============================
// Resolve register file conflict
// ==============================

/**
 * Determine reg file of given register.
 */
RegTag regFileOf(Reg r) {
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


// =============================
// Satisfy VideoCore constraints
// =============================

// Transform an instruction sequence to satisfy various VideoCore
// constraints, including:
//
//   1. fill branch delay slots with NOPs;
//
//   2. introduce accumulators for operands mapped to the same
//      register file;
//
//   3. introduce accumulators for horizontal rotation operands;
//
//   4. insert NOPs to account for data hazards: a destination
//      register (assuming it's not an accumulator) cannot be read by
//      the next instruction.

void satisfy(Seq<Instr>* instrs) {
  assert(instrs != nullptr);

  // New instruction sequence
  Seq<Instr> newInstrs0(instrs->size() * 2);
  Seq<Instr> newInstrs1(instrs->size() * 2);

  // Apply passes
  insertMoves(*instrs, &newInstrs0);
  insertNops(newInstrs0, newInstrs1);
  instrs->clear();
  removeVPMStall(newInstrs1, *instrs);
}

}  // namespace V3DLib
