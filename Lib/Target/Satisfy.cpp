#include "Satisfy.h"
#include <assert.h>
#include <stdio.h>
#include "Support/Platform.h"
#include "Liveness/Liveness.h"
#include "Target/instr/Mnemonics.h"
#include "Liveness/UseDef.h"

namespace V3DLib {
namespace {

bool hasRegFileConflict(Instr const &instr) {
  if (instr.tag == ALU && instr.ALU.srcA.is_reg() && instr.ALU.srcB.is_reg()) {
    int rfa = instr.ALU.srcA.reg().regfile();
    int rfb = instr.ALU.srcB.reg().regfile();

    if (rfa != NONE && rfb != NONE) {
      return (rfa == rfb) && !(instr.ALU.srcA == instr.ALU.srcB);
    }
  }

  return false;
}


/**
 * First pass for satisfy constraints: insert move-to-accumulator instructions
 */
Instr::List insertMoves_vc4(Instr::List &instrs) {
  assert(Platform::compiling_for_vc4());  // Not an issue for v3d

  Instr::List newInstrs(instrs.size() * 2);

  for (int i = 0; i < instrs.size(); i++) {
    using namespace Target::instr;
    Instr instr = instrs[i];

    if (instr.tag == ALU && instr.ALU.srcA.is_imm() &&
        instr.ALU.srcB.is_reg() && instr.ALU.srcB.reg().regfile() == REG_B) {
      // Insert moves for an operation with a small immediate whose
      // register operand must reside in reg file B.
      newInstrs << mov(ACC0, instr.ALU.srcB)
                << instr.clone().src_b(ACC0);
    } else if (instr.tag == ALU && instr.ALU.srcB.is_imm() &&
               instr.ALU.srcA.is_reg() && instr.ALU.srcA.reg().regfile() == REG_B) {
      // Insert moves for an operation with a small immediate whose
      // register operand must reside in reg file B.
      newInstrs << mov(ACC0, instr.ALU.srcA)
                << instr.clone().src_a(ACC0);
    } else if (hasRegFileConflict(instr)) {
      // Insert moves for operands that are mapped to the same reg file.
      //
      // When an instruction uses two (different) registers that are mapped
      // to the same register file, then remap one of them to an accumulator.
      newInstrs << mov(ACC0, instr.ALU.srcA)
                << instr.clone().src_a(ACC0);
    } else {
      newInstrs << instr;
    }
    
  }

  return newInstrs;
}


Instr::List insertMoves(Instr::List &instrs) {
  Instr::List newInstrs(instrs.size() * 2);

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    if (instr.isRot()) {
      // Insert moves for horizontal rotate operations
      using namespace Target::instr;

      newInstrs << mov(ACC0, instr.ALU.srcA);

      auto instr2 = instr.clone().src_a(ACC0);

      if (instr.ALU.srcB.is_reg()) {
        newInstrs << mov(ACC5, instr.ALU.srcB);
        instr2.src_b(ACC5);
      }

      newInstrs << Instr::nop()
                << instr2;
    } else {
      newInstrs << instr;
    }
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
 * Insert NOPs between VPM setup and VPM read, if needed
 *
 * Replaces VPM_STALL instructions with NOPs.
 */
Instr::List removeVPMStall(Instr::List &instrs) {
  Instr::List newInstrs(instrs.size() * 2);

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];

    if (instr.tag != VPM_STALL) {
      newInstrs << instr;
      continue;
    }

    int numNops = 3;  // Number of nops to insert

    for (int j = 1; j <= 3; j++) {
      if ((i + j) >= instrs.size()) break;
      Instr next = instrs[i+j];

      if (next.tag == LAB) break;
      if (next.is_src_reg(Target::instr::VPM_READ)) break;

      numNops--;
    }

    for (int j = 0; j < numNops; j++)
      newInstrs << Instr::nop();
  }

  return newInstrs;
}

}  // anon namespace


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

  if (Platform::compiling_for_vc4()) {
    newInstrs = insertMoves_vc4(newInstrs);
  }

  newInstrs = insertNops(newInstrs);
  instrs = removeVPMStall(newInstrs);
}

}  // namespace V3DLib
