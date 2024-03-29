#include "SourceTranslate.h"
#include <iostream>
#include "Support/basics.h"
#include "Support/Timer.h"
#include "Source/Translate.h"
#include "Source/Stmt.h"
#include "Liveness/Liveness.h"
#include "Target/Subst.h"
#include "vc4/DMA/DMA.h"
#include "Target/instr/Mnemonics.h"
#include "Common/CompileData.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace v3d {

Instr::List SourceTranslate::store_var(Var dst_addr, Var src) {
  using namespace V3DLib::Target::instr;
  Instr::List ret;

  Reg srcData(dst_addr);
  Reg srcAddr(src);

  ret << mov(TMUD, srcAddr)
      << mov(TMUA, srcData)
      << tmuwt();

  ret.front().comment("store_var v3d");

  return ret;
}


void SourceTranslate::regAlloc(Instr::List &instrs) {
  //Timer t1("regAlloc", true);
  int numVars = VarGen::count();

  //Timer t2("regAlloc optimize");
  Liveness::optimize(instrs, numVars);
  //t2.end();

  // Step 0 - Perform liveness analysis
  //Timer t3("regAlloc compute");
  Liveness live(numVars);
  live.compute(instrs);
  //t3.end();

  // Step 2 - For each variable, determine all variables ever live at the same time
  //Timer t4("regAlloc liveWith");
  LiveSets liveWith(numVars);
  liveWith.init(instrs, live);
  //t4.end();

  //Timer t5("regAlloc Allocate reg to var");

  // Step 3 - Allocate a register to each variable
  for (int i = 0; i < numVars; i++) {
    if (live.reg_usage()[i].reg.tag != NONE) continue;

    auto possible = liveWith.possible_registers(i, live.reg_usage());

    live.reg_usage()[i].reg.tag = REG_A;
    RegId regId = LiveSets::choose_register(possible, false);

    if (regId < 0) {
      std::string buf = "v3d regAlloc(): register allocation failed for target instruction ";
      buf << i << ": " << instrs[i].mnemonic();
      error(buf, true);
    } else {
      live.reg_usage()[i].reg.regId = regId;
    }
  }

  //t5.end();

  compile_data.allocated_registers_dump = live.reg_usage().dump(true);

  // Step 4 - Apply the allocation to the code
  //Timer t6("regAlloc allocate_registers");
  allocate_registers(instrs, live.reg_usage());
  //t6.end();
}


Instr label(Label in_label) {
  Instr instr;
  instr.tag = LAB;
  instr.label(in_label);

  return instr;
}


/**
 * Add extra initialization code after uniform loads
 */
void add_init(Instr::List &code) {
  using namespace V3DLib::Target::instr;

  int insert_index = code.tag_index(INIT_BEGIN);
  assertq(insert_index >= 0, "Expecting init begin marker");

  Instr::List ret;
  Label endifLabel = freshLabel();

  // Determine the qpu index for 'current' QPU
  // This is derived from the thread index. 
  //
  // Broadly:
  //
  // If (numQPUs() == 8)  // Alternative is 1, then qpu num initalized to 0 is ok
  //   me() = (thread_index() >> 2) & 0b1111;
  // End
  //
  // This works because the thread indexes are consecutive for multiple reserved
  // threads. It's probably also the reason why you can select only 1 or 8 (max)
  // threads, otherwise there would be gaps in the qpu id.
  //
  ret << mov(rf(RSV_QPU_ID), 0)           // not needed, already init'd to 0. Left here to counter future brainfarts
      << sub(ACC0, rf(RSV_NUM_QPUS), 8).pushz()
      << branch(endifLabel).allzc()       // nop()'s added downstream
      << mov(ACC0, QPU_ID)
      << shr(ACC0, ACC0, 2)
      << band(rf(RSV_QPU_ID), ACC0, 15)
      << label(endifLabel)

      << add_uniform_pointer_offset(code);

  code.insert(insert_index + 1, ret);  // Insert init code after the INIT_BEGIN marker
}


/**
 * @return true if statement handled, false otherwise
 */
bool SourceTranslate::stmt(Instr::List &seq, Stmt::Ptr s) {
  if (DMA::Stmt::is_dma_tag(s->tag)) {
    fatal("VPM and DMA reads and writes can not be used for v3d");
    return true;
  }

  return false;
}

}  // namespace v3d
}  // namespace V3DLib
