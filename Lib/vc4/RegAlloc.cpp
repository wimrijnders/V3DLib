#include "RegAlloc.h"
#include <stdio.h>
#include "Support/basics.h"  // fatal()
#include "Source/Syntax.h"
#include "Target/Syntax.h"
#include "Target/Subst.h"
#include "SourceTranslate.h"

namespace QPULib {

namespace {

/**
 * For each variable, determine a preference for register file A or B.
 */
void regalloc_determine_regfileAB(Seq<Instr> *instrs, int *prefA, int *prefB, int n) {
	//breakpoint

  for (int i = 0; i < n; i++) prefA[i] = prefB[i] = 0;

  for (int i = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    Reg ra, rb;
    if (getTwoUses(instr, &ra, &rb) && ra.tag == REG_A && rb.tag == REG_A) {
      RegId x = ra.regId;
      RegId y = rb.regId;
      if (prefA[x] > prefA[y] || prefB[y] > prefB[x])
        { prefA[x]++; prefB[y]++; }
      else
        { prefA[y]++; prefB[x]++; }
    }
    else if (instr.tag == ALU &&
             instr.ALU.srcA.tag == REG &&
             instr.ALU.srcA.reg.tag == REG_A &&
             instr.ALU.srcB.tag == IMM) {
      prefA[instr.ALU.srcA.reg.regId]++;
    }
    else if (instr.tag == ALU &&
             instr.ALU.srcB.tag == REG &&
             instr.ALU.srcB.reg.tag == REG_A &&
             instr.ALU.srcA.tag == IMM) {
      prefA[instr.ALU.srcB.reg.regId]++;
    }
  }
}

}  // anon namespace

// ============================================================================
// Register allocation
// ============================================================================

namespace vc4 {

void regAlloc(CFG* cfg, Seq<Instr>* instrs) {
  // Step 0
  // Perform liveness analysis
  Liveness live(*cfg);
  live.compute(instrs);

  // Step 1
  // For each variable, determine a preference for register file A or B.
  int n = getFreshVarCount();
  int* prefA = new int [n];
  int* prefB = new int [n];

	regalloc_determine_regfileAB(instrs, prefA, prefB, n);

  // Step 2
  // For each variable, determine all variables ever live at same time
  LiveSets liveWith(n);
	liveWith.init(instrs, live);

  // Step 3
  // Allocate a register to each variable
  RegTag prevChosenRegFile = REG_B;
  std::vector<Reg> alloc(n);
  for (int i = 0; i < n; i++) alloc[i].tag = NONE;

  for (int i = 0; i < n; i++) {
		auto possibleA = liveWith.possible_registers(i, alloc);
		auto possibleB = liveWith.possible_registers(i, alloc, REG_B);

    // Find possible register in each register file
    RegId chosenA = LiveSets::choose_register(possibleA, false);
    RegId chosenB = LiveSets::choose_register(possibleB, false);

    // Choose a register file
    RegTag chosenRegFile;
    if (chosenA < 0 && chosenB < 0) {
      fatal("QPULib: register allocation failed, insufficient capacity");
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

  // Step 4
  // Apply the allocation to the code
  for (int i = 0; i < instrs->numElems; i++) {
		auto &useDefSet = liveWith.useDefSet;
    Instr* instr = &instrs->elems[i];

    useDef(*instr, &useDefSet);
    for (int j = 0; j < useDefSet.def.numElems; j++) {
      RegId r = useDefSet.def.elems[j];
      RegTag tmp = alloc[r].tag == REG_A ? TMP_A : TMP_B;
      renameDest(instr, REG_A, r, tmp, alloc[r].regId);
    }
    for (int j = 0; j < useDefSet.use.numElems; j++) {
      RegId r = useDefSet.use.elems[j];
      RegTag tmp = alloc[r].tag == REG_A ? TMP_A : TMP_B;
      renameUses(instr, REG_A, r, tmp, alloc[r].regId);
    }
    substRegTag(instr, TMP_A, REG_A);
    substRegTag(instr, TMP_B, REG_B);
  }

  // Free memory
  delete [] prefA;
  delete [] prefB;
}

}  // namespace vc4; 
}  // namespace QPULib
