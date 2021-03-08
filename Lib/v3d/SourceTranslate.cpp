#include "SourceTranslate.h"
#include "Support/basics.h"
#include "Source/Translate.h"
#include "Source/Stmt.h"
#include "Target/Liveness.h"
#include "Target/Subst.h"
#include "vc4/DMA/DMA.h"
#include "Target/instr/Instructions.h"
#include "Common/CompileData.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace v3d {

Instr::List SourceTranslate::store_var(Var dst_addr, Var src) {
  using namespace V3DLib::Target::instr;

  Instr::List ret;

  Reg srcData = srcReg(dst_addr);
  Reg srcAddr = srcReg(src);

  if (src.tag() == ELEM_NUM) {
    // Why would you ever do this?
    breakpoint  // check if ever used

    //TODO: is ACC0 safe here?
    assert(srcAddr == ELEM_ID);
    ret << mov(ACC0, ELEM_ID)
        << mov(TMUD, ACC0);
  } else {
    ret << mov(TMUD, srcAddr);
  }

  ret << mov(TMUA, srcData)
      << tmuwt();

  return ret;
}


void SourceTranslate::regAlloc(CFG* cfg, Instr::List &instrs) {
  int numVars = getFreshVarCount();

  // Step 0 - Perform liveness analysis
  Liveness live(*cfg);
  live.compute(instrs);
  assert(instrs.size() == live.size());

  // Step 2 - For each variable, determine all variables ever live at the same time
  LiveSets liveWith(numVars);
  liveWith.init(instrs, live);

  // Step 3 - Allocate a register to each variable
  std::vector<Reg> alloc(numVars);
  for (int i = 0; i < numVars; i++) alloc[i].tag = NONE;

  // Allocate registers to the variables
  for (int i = 0; i < numVars; i++) {
    auto possible = liveWith.possible_registers(i, alloc);

    alloc[i].tag = REG_A;
    RegId regId = LiveSets::choose_register(possible, false);

    if (regId < 0) {
      std::string buf = "v3d regAlloc(): register allocation failed for target instruction ";
      buf << i << ": " << instrs[i].mnemonic();
      error(buf, true);
    } else {
      alloc[i].regId = regId;
    }
  }

  compile_data.allocated_registers(alloc);

  // Step 4 - Apply the allocation to the code
  for (int i = 0; i < instrs.size(); i++) {
    auto &useDefSet = liveWith.useDefSet;
    Instr &instr = instrs.get(i);

    useDef(instr, &useDefSet);
    for (int j = 0; j < useDefSet.def.size(); j++) {
      RegId r = useDefSet.def[j];
      renameDest(&instr, REG_A, r, TMP_A, alloc[r].regId);
    }
    for (int j = 0; j < useDefSet.use.size(); j++) {
      RegId r = useDefSet.use[j];
      renameUses(&instr, REG_A, r, TMP_A, alloc[r].regId);
    }
    substRegTag(&instr, TMP_A, REG_A);
  }
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
