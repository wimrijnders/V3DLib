#include "RegAlloc.h"
#include <stdio.h>
#include <iostream>
#include "Support/basics.h"
#include "Target/Subst.h"
#include "SourceTranslate.h"
#include "Common/CompileData.h"

namespace V3DLib {
namespace {

/**
 * Return true if given instruction has two register operands.
 */
bool getTwoUses(Instr instr, Reg* r1, Reg* r2) {
  if (instr.tag == ALU && instr.ALU.srcA.is_reg() && instr.ALU.srcB.is_reg()) {
    *r1 = instr.ALU.srcA.reg;
    *r2 = instr.ALU.srcB.reg;
    return true;
  }

  return false;
}


/**
 * For each variable, determine a preference for register file A or B.
 */
void regalloc_determine_regfileAB(Instr::List &instrs, int *prefA, int *prefB, int n) {
  for (int i = 0; i < n; i++) prefA[i] = prefB[i] = 0;

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];
    Reg ra, rb;
    if (getTwoUses(instr, &ra, &rb) && ra.tag == REG_A && rb.tag == REG_A) {
      RegId x = ra.regId;
      RegId y = rb.regId;
      if (prefA[x] > prefA[y] || prefB[y] > prefB[x]) {
        prefA[x]++; prefB[y]++;
      } else {
        prefA[y]++; prefB[x]++;
      }
    } else if (instr.tag == ALU &&
               instr.ALU.srcA.is_reg() && instr.ALU.srcA.reg.tag == REG_A &&
               instr.ALU.srcB.is_imm()) {
      prefA[instr.ALU.srcA.reg.regId]++;
    } else if (instr.tag == ALU &&
               instr.ALU.srcB.is_reg() && instr.ALU.srcB.reg.tag == REG_A &&
               instr.ALU.srcA.is_imm()) {
      prefA[instr.ALU.srcB.reg.regId]++;
    }
  }
}

}  // anon namespace


// ============================================================================
// Register allocation
// ============================================================================

namespace vc4 {

void regAlloc(CFG *cfg, Instr::List &instrs) {
  assert(cfg != nullptr);
  int numVars = getFreshVarCount();

  // Step 0 - Perform liveness analysis
  Liveness live(*cfg);
  live.compute(instrs);


  // Step 1 - For each variable, determine a preference for register file A or B.
  int* prefA = new int [numVars];
  int* prefB = new int [numVars];

  regalloc_determine_regfileAB(instrs, prefA, prefB, numVars);

  // Step 2 - For each variable, determine all variables ever live at same time
  LiveSets liveWith(numVars);
  liveWith.init(instrs, live);

  std::vector<Reg> alloc(numVars);
  for (int i = 0; i < numVars; i++) alloc[i].tag = NONE;

  // Introduce accumulators where possible
  compile_data.num_accs_introduced = introduceAccum(live, instrs, alloc);

  // Step 3 - Allocate a register to each variable
  RegTag prevChosenRegFile = REG_B;

  for (int i = 0; i < numVars; i++) {
    if (alloc[i].tag != NONE) continue;

    auto possibleA = liveWith.possible_registers(i, alloc);
    auto possibleB = liveWith.possible_registers(i, alloc, REG_B);

    // Find possible register in each register file
    RegId chosenA = LiveSets::choose_register(possibleA, false);
    RegId chosenB = LiveSets::choose_register(possibleB, false);

    // Choose a register file
    RegTag chosenRegFile;
    if (chosenA < 0 && chosenB < 0) {
      error("regAlloc(): register allocation failed, insufficient capacity", true);
    }
    else if (chosenA < 0) chosenRegFile = REG_B;
    else if (chosenB < 0) chosenRegFile = REG_A;
    else {
      if (prefA[i] > prefB[i]) chosenRegFile = REG_A;
      else if (prefA[i] < prefB[i]) chosenRegFile = REG_B;
      else chosenRegFile = prevChosenRegFile == REG_A ? REG_B : REG_A;
    }
    prevChosenRegFile = chosenRegFile;

    // Finally, allocate a register to the variable
    alloc[i].tag = chosenRegFile;
    alloc[i].regId = chosenRegFile == REG_A ? chosenA : chosenB;
  }

  compile_data.allocated_registers(alloc);

  // Step 4 - Apply the allocation to the code
  for (int i = 0; i < instrs.size(); i++) {
    auto &useDefSet = liveWith.useDefSet;
    Instr &instr = instrs.get(i);

    useDef(instr, &useDefSet);

    for (int j = 0; j < useDefSet.def.size(); j++) {
      RegId r = useDefSet.def[j];
      RegTag tmp = alloc[r].tag == REG_A ? TMP_A : TMP_B;

      Reg current(REG_A, r);
      Reg replace_with(tmp, alloc[r].regId);
      renameDest(instr, current, replace_with);
    }

    for (int j = 0; j < useDefSet.use.size(); j++) {
      RegId r = useDefSet.use[j];
      RegTag tmp = alloc[r].tag == REG_A ? TMP_A : TMP_B;

      Reg current(REG_A, r);
      Reg replace_with(tmp, alloc[r].regId);
      renameUses(instr, current, replace_with);
    }
    substRegTag(&instr, TMP_A, REG_A);
    substRegTag(&instr, TMP_B, REG_B);
  }

  // Free memory
  delete [] prefA;
  delete [] prefB;
}

}  // namespace vc4; 
}  // namespace V3DLib
