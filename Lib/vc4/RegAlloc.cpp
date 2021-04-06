#include "RegAlloc.h"
#include <stdio.h>
#include <iostream>
#include "Support/basics.h"
#include "Support/Timer.h"
#include "Target/Subst.h"
#include "SourceTranslate.h"
#include "Common/CompileData.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace {

/**
 * Return true if given instruction has two register operands.
 */
bool getTwoUses(Instr instr, Reg* r1, Reg* r2) {
  if (instr.tag == ALU && instr.ALU.srcA.is_reg() && instr.ALU.srcB.is_reg()) {
    *r1 = instr.ALU.srcA.reg();
    *r2 = instr.ALU.srcB.reg();
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
               instr.ALU.srcA.is_reg() && instr.ALU.srcA.reg().tag == REG_A &&
               instr.ALU.srcB.is_imm()) {
      prefA[instr.ALU.srcA.reg().regId]++;
    } else if (instr.tag == ALU &&
               instr.ALU.srcB.is_reg() && instr.ALU.srcB.reg().tag == REG_A &&
               instr.ALU.srcA.is_imm()) {
      prefA[instr.ALU.srcB.reg().regId]++;
    }
  }
}


struct RegTypeCount {
  static int const NumRegTypes = (TMP_B + 1);

  RegTypeCount() {
    for (int i = 0; i < NumRegTypes; i++) {
      list[i] = 0;
    }
  }

  bool safe_for_regalloc() const {
    return list[REG_B] == 0 && list[TMP_A] == 0 && list[TMP_B] ==0;
  }

  /**
   * Debug function to display the register types count of an instruction list
   */
  std::string dump() {
    std::string ret = "Used register types in instruction list:\n";
  
    ret << "  REG_A    : " << list[REG_A]   << "\n"
        << "  REG_B    : " << list[REG_B]   << "\n"
        << "  ACC      : " << list[ACC]     << "\n"
        << "  SPECIAL  : " << list[SPECIAL] << "\n"
        << "  NONE     : " << list[NONE]    << "\n"
        << "  TMP_A    : " << list[TMP_A]   << "\n"
        << "  TMP_B    : " << list[TMP_B]   << "\n"
        << "\n";

    return ret;
  }

  int list[NumRegTypes];
};


/**
 * Determine the register types count in an instruction list
 */
RegTypeCount count_reg_types(Instr::List &instrs) {
  RegTypeCount reg_types;

  for (int i = 0; i < instrs.size(); i++) {
    UseDefReg def_regs;
    def_regs.set_used(instrs[i]);

    for (int j = 0; j < def_regs.use.size(); j++) {
      reg_types.list[def_regs.use[j].tag]++;
    }
    for (int j = 0; j < def_regs.def.size(); j++) {
      reg_types.list[def_regs.def[j].tag]++;
    }
  }

  return reg_types;
}

}  // anon namespace


// ============================================================================
// Register allocation
// ============================================================================

namespace vc4 {

/**
 * The incoming instruction list has all variables assigned as
 * registers in register file A, with the index set to the variable index.
 *
 * The list can contain predefined accumulators, SPECIAL registers and NONE.
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * Profile timing 20210406
 *
 *   vc4 DFT profiling in unit test, -d=272
 *   Conclusion: liveWith.init() is the main timing hog
 *
 *  - Matrix mult
 *      optimize                :  1.399301s
 *      compute                 :  0.872933s
 *      liveWith                :  5.927731s  <--
 *      allocate_reg            :  0.007310s
 *      apply allocate_registers:  0.003695s
 *      total                   :  8.217817s
 *
 *  - Inline complex
 *      optimize                :  9.845932s
 *      compute                 :  5.468031s
 *      liveWith                : 62.917497s  <--
 *      allocate_reg            :  0.031919s
 *      apply allocate_registers:  0.017052s
 *      Total                   : 78.310163s
 *
 *  - Inline float
 *      optimize                :  5.230446s
 *      compute                 :  2.488139s
 *      liveWith                : 19.684118s  <--
 *      allocate_reg            :  0.019152s
 *      apply allocate_registers:  0.015658s
 *      Total                   : 27.464903s
 *
 *  - Replacing `V3DLib::Set` with `std::set`:
 *
 *    Conclusion: still the biggest timing hog, but much better
 *
 *  - Matrix mult
 *      liveWith                :  2.272735s
 *  - Inline complex
 *      liveWith                : 15.885858s
 *  - Inline float
 *    liveWith                  :  6.156424s
 */
void regAlloc(Instr::List &instrs) {
  assert(count_reg_types(instrs).safe_for_regalloc());
//  Timer t1("vc4 regAlloc", true);
  //std::cout << count_reg_types(instrs).dump() << std::endl;

  int numVars = VarGen::count();

//{
//  Timer t("vc4 regAlloc optimize", true);
  Liveness::optimize(instrs, numVars);
//}

  // Step 0 - Perform liveness analysis
  Liveness live(numVars);
//{
//  Timer t("vc4 regAlloc compute", true);
  live.compute(instrs);
//}


  // Step 1 - For each variable, determine a preference for register file A or B.
  int *prefA = new int [numVars];
  int *prefB = new int [numVars];

  regalloc_determine_regfileAB(instrs, prefA, prefB, numVars);

  // Step 2 - For each variable, determine all variables ever live at same time
  LiveSets liveWith(numVars);
//{
//  Timer t("vc4 regAlloc liveWith", true);
  liveWith.init(instrs, live);
//}

  // Step 3 - Allocate a register to each variable
  RegTag prevChosenRegFile = REG_B;

//{
//  Timer t("vc4 regAlloc allocate_reg", true);

  for (int i = 0; i < numVars; i++) {
    if (live.reg_usage()[i].reg.tag != NONE) continue;
    if (live.reg_usage()[i].unused()) continue;

    auto possibleA = liveWith.possible_registers(i, live.reg_usage());
    auto possibleB = liveWith.possible_registers(i, live.reg_usage(), REG_B);

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
    live.reg_usage()[i].reg = Reg(chosenRegFile, (chosenRegFile == REG_A)? chosenA : chosenB);
  }
//}
  
  compile_data.allocated_registers_dump = live.reg_usage().dump(true);
  //std::cout << count_reg_types(instrs).dump() << std::endl;

  // Step 4 - Apply the allocation to the code
//{
//  Timer t("vc4 regAlloc apply allocate_registers", true);
  allocate_registers(instrs, live.reg_usage());
//}

  //std::cout << instrs.check_acc_usage() << std::endl;

  // Free memory
  delete [] prefA;
  delete [] prefB;
}

}  // namespace vc4; 
}  // namespace V3DLib
