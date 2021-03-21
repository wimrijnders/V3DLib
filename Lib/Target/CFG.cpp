#include "CFG.h"
#include <assert.h>

namespace V3DLib {


/**
 * Check if given instruction only passes to the next instruction
 *
 * @return true if instruction has only one successor, and that is 
 *         the next instruction, false otherwise
 */
bool CFG::is_regular(InstrId i) const {
  auto const &item = (*this)[i];
  return (item.size() == 1 && item[0] == (i + 1));
}


std::string CFG::dump() const {
  using ::operator<<;

  std::string ret;

  ret << "Instruction length: " << size() << "\n";

  for (int i = 0; i < size(); ++i) {
    auto const &item = (*this)[i];
    if (is_regular(i)) continue;

    ret << i << ": " << item << "\n";
  } 

  ret << dump_blocks();
  return ret;
}


std::string CFG::dump_blocks() const {
  std::string ret;

  ret << "\nBlocks:\n";

  // Make a kind of header
  int tabs = 0;
  for (int i = 0; i < (int) blocks.size(); ++i) {
    if (i % 10 == 0) {
      ret << i;

     if (i < 10)        tabs = 0;
     else if (i < 100)  tabs = 1;
     else if (i < 1000) tabs = 2;
     else assert(false);  // Expand this list if this happens
    } else {
      if (tabs > 0) {
        tabs--;
      } else {
        ret << " ";
     }
    }
  }

  ret << "\n";

  for (int i = 0; i < (int) blocks.size(); ++i) {
    assert(blocks[i] < 10);  // If this happens, we need to adjust the display (not critical)
    ret << blocks[i];
  }

  ret << "\n";

  return ret;
}


/**
 * Build a CFG for a given instruction sequence.
 */
void CFG::build(Instr::List &instrs) {
  assert(empty());
  set_size(instrs.size());

  // ----------
  // First pass
  // ----------
  //
  // 1. Each instruction is a successor of the previous
  //    instruction, unless the previous instruction
  //    is an unconditional jump or halt instruction.
  //
  // 2. Compute a mapping from labels to instruction ids.

  // Number of labels in program
  int numLabels = getFreshLabelCount();

  // Mapping from labels to instruction ids
  InstrId* labelMap = new InstrId [numLabels];

  // Initialise label mapping
  for (int i = 0; i < numLabels; i++)
    labelMap[i] = -1;

  for (int i = 0; i < instrs.size(); i++) {
    // Get instruction
    Instr instr = instrs[i];

    // Is it an unconditional jump?
    bool uncond = instr.tag == BRL && instr.BRL.cond.tag == COND_ALWAYS;

    // Is it a final instruction?
    bool end = instr.tag == END || i+1 == instrs.size();

    // Add successor
    if (!(uncond || end))
      (*this)[i].insert(i+1);

    // Remember location of each label
    if (instr.tag == LAB) {
      assert(instr.label() >= 0 && instr.label() < numLabels);
      labelMap[instr.label()] = i;
    }
  }

  // -----------
  // Second pass
  // -----------
  //
  // Add a successor for each conditional jump.

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];
    if (instr.tag == BRL) {
      assert(labelMap[instr.BRL.label] >= 0);
      (*this)[i].insert(labelMap[instr.BRL.label]);
    }
  }

  // Free memory
  delete [] labelMap;

  build_blocks();
}


void CFG::build_blocks() {
  blocks.resize(size());

  for (int i = 0; i < (int) blocks.size(); i++) {
    blocks[i] = 0;
  }


  int block_index = 0;

  for (int i = 0; i < (int) size(); i++) {
    if (is_regular(i)) continue;
    auto const &item = (*this)[i];

    for (int j = 0; j < (int) item.size(); j++) {
      int jump_instr = item[j];

      if (jump_instr == i + 1) continue;  // Skip regular successor
      if (jump_instr <= i) continue;  // Skip jumps backward for now. TODO investigate if these should be noted here

      block_index++;
      for (int k = i + 1; k < (int) jump_instr; k++) {
        blocks[k] = block_index;
      }
    }
  }
}

}  // namespace V3DLib
