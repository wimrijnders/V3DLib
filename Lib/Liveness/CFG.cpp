#include "CFG.h"
#include <iostream>
#include "Support/basics.h"

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class CFG::Blocks
///////////////////////////////////////////////////////////////////////////////

void CFG::Blocks::build(CFG const &cfg) {
  list.resize(cfg.size());

  for (int i = 0; i < (int) list.size(); i++) {
    list[i] = 0;
  }

  int block_index = 0;

  for (int i = 0; i < (int) cfg.size(); i++) {
    if (cfg.is_regular(i)) continue;

    for (auto jump_instr : cfg[i]) {
      if (jump_instr == i + 1) continue;  // Skip regular successor
      if (jump_instr <= i) continue;  // Skip jumps backward for now. TODO investigate if these should be noted here

      block_index++;
      for (int k = i + 1; k < (int) jump_instr; k++) {
        list[k] = block_index;
      }
    }
  }

  add_ranges();
}


void CFG::Blocks::clear() {
  list.clear();
  ranges.clear();
}


void CFG::Blocks::add_ranges() {
  assert(!list.empty());
  assert(ranges.empty());

  for (int i = 0; i < (int) list.size(); i++) {
    int block_num = list[i];
    assert(block_num >= 0);

    if ((int) ranges.size() < block_num + 1) {
      ranges.resize(block_num + 1);
    }

    ranges[block_num].add(i);
  }
}


std::string CFG::Blocks::dump() const {
  std::string ret;

  auto tabs = [] (int i) -> int {
    int ret = -1;

   if      (i < 10)     ret = 1;
   else if (i < 100)    ret = 2;
   else if (i < 1000)   ret = 3;
   else if (i < 10000)  ret = 4;
   else if (i < 100000) ret = 5;
   else {
     std::string msg =  "Blocks::dump() i: ";
     msg << i;
     assertq(false, msg);  // Expand this list if this happens
   }

    return ret;
  };


  int const block_tabs = tabs(max_block_num());

  ret << "\nBlocks:\n";

  // Make a kind of header
  // Show the index every LINE_MAX steps
  int const LINE_MAX = (block_tabs == 1)?210:110;

  auto header = [&ret, this, tabs, block_tabs] (int min, int max) {
    int list_size = (int) this->list.size();
    if (max > list_size) {
      max = list_size;
    }

    int last_width = -1;
    for (int i = 0; i < (max - min); ++i) {
      if (i % 10 != 0) continue;

      if (i != 0) {
        int tmp_width = 10*block_tabs - last_width;
        ret << tabbed(tmp_width, " ");
      }

      int width = tabs(i + min);
      ret << tabbed(width, i + min);
      last_width = width;
    }
  };


  for (int i = 0; i < (int) list.size(); ++i) {
    if (i % LINE_MAX == 0) {
      ret << "\n";
      header(i, i + LINE_MAX);
      ret << "\n";
    }
    ret << tabbed(block_tabs, list[i]);
  }

  ret << "\n\nRanges:\n";
  for (int i = 0; i < (int) ranges.size(); ++i) {
    ret << "  " << i << ": " << ranges[i].dump() << "\n";
  }

  ret << "\n";

  if (block_tabs > 2) {
    std::cout << ret << std::endl;
    breakpoint  // Check what happens in this case
  }

  return ret;
}


/**
 * Find the last line num which has the same block as `line_num`.
 */
int CFG::Blocks::end(InstrId line_num) const {
 int ret = list[line_num];

 for (int j = line_num + 1; j < (int) list.size(); j++) {
   if (list[line_num] == list[j]) {
     ret = j;
   }
 }

 return ret;
}


/**
 * Check if given cur_block is same as or embedded in parent_block
 *
 * This method assumes that all blocks are numbered uniquely.
 * Assumption should hold (it does at time of writing), but is not enforced.
 */
bool CFG::Blocks::block_in(int cur_block, int parent_block) const {
  assert(!list.empty());                                                    // Total paranoia
  assert(!ranges.empty());
  assert(0 <= cur_block    && cur_block    < (int) ranges.size());
  assert(0 <= parent_block && parent_block < (int) ranges.size());
  assert(list[0] == 0);                                                     // Double-check that top block is zero.
  assert(ranges[0].first() == 0 && ranges[0].last() == ((int)list.size() - 1));  // idem

  if (cur_block == parent_block) return true;

  if (ranges[parent_block].is_embedded(ranges[cur_block])) {
    breakpoint
    return true;
  }

  return false;
}


int CFG::Blocks::max_block_num() const {
  int ret = -1;

  for (int i = 0; i < (int) list.size(); ++i) {
    if (ret == -1 || ret < list[i]) {
      ret = list[i];
    }
  }

  assert(ret != -1 );
  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Class CFG
///////////////////////////////////////////////////////////////////////////////

/**
 * Check if given instruction only passes to the next instruction
 *
 * @return true if instruction has only one successor, and that is 
 *         the next instruction, false otherwise
 */
bool CFG::is_regular(InstrId i) const {
  auto const &item = (*this)[i];
  return (item.size() == 1 && item.first() == (i + 1));
}


std::string CFG::dump() const {
  std::string ret;

  ret << "Instruction length: " << size() << "\n";

  for (int i = 0; i < (int) size(); ++i) {
    auto const &item = (*this)[i];
    if (is_regular(i)) continue;

    ret << i << ": " << item.dump() << "\n";
  } 

  ret << blocks.dump();
  return ret;
}


/**
 * Build a CFG for a given instruction sequence.
 */
void CFG::build(Instr::List &instrs) {
  assert(empty());
  //set_size(instrs.size());
  resize(instrs.size());

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
    bool uncond = instr.tag == BRL && instr.branch_cond().is_always();

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
      assert(labelMap[instr.branch_label()] >= 0);
      (*this)[i].insert(labelMap[instr.branch_label()]);
    }
  }

  // Free memory
  delete [] labelMap;

  blocks.build(*this);
}


/**
 * Returns the block of given instruction line
 */
int CFG::block_at(InstrId line_num) const {
  return blocks.list[line_num];
}


void CFG::clear() {
  Parent::clear();
  blocks.clear();
}


/**
 * Check if given block is same as or parent of block at line_num
 */
bool CFG::is_parent_block(InstrId line_num, int block) const {
  int cur_block = blocks.list[line_num];

  return blocks.block_in(cur_block, block);
}

}  // namespace V3DLib
