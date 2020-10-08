#ifndef _QPULIB_REMOVELABELS_H_
#define _QPULIB_REMOVELABELS_H_

#include "Target/Syntax.h"
#include "Target/CFG.h"
#include "Target/Liveness.h"
#include "Common/Seq.h"

namespace QPULib {

/**
 * Remove labels
 *
 * Remove all labels, replacing absolute branch-label instructions
 * with relative branch-target instructions.
 *
 * @param instr  sequence of instructions in which to replace labels.
 */
template<typename Instructions>
void removeLabels(Instructions &instrs) {
  Instructions newInstrs;

  // The number of labels in the instruction sequence
  int numLabels = getFreshLabelCount();

  // A mapping from labels to instruction ids
  InstrId* labels = new InstrId[numLabels];

  // Initialise label mapping
  for (int i = 0; i < numLabels; i++)
    labels[i] = -1;

  // First, remove labels, remembering the index of the instruction
  // pointed to by each label.
  for (int i = 0, j = 0; i < instrs.size(); i++) {
    auto &instr = instrs[i];

    if (instr.is_label()) {
      labels[instr.label()] = j;
    } else {
      newInstrs << instr;
      j++;
    }
  }

  // Second, convert branch-label instructions.
  for (int i = 0; i < newInstrs.size(); i++) {
    auto &instr = newInstrs[i];

    if (instr.is_branch_label()) {
			// Convert branch-to-label to branch-to-target
      assert(0 <= instr.branch_label() && instr.branch_label() < numLabels);
      int dest = labels[instr.branch_label()];
      assert(dest >= 0);

			instr.label_to_target(dest - i);  // pass in offset to label from current instruction
    }
  }

	breakpoint
	instrs = newInstrs;

  delete [] labels;
}

}  // namespace QPULib

#endif  // _QPULIB_REMOVELABELS_H_
