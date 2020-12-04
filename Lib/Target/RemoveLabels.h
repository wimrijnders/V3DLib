#ifndef _V3DLIB_REMOVELABELS_H_
#define _V3DLIB_REMOVELABELS_H_
#include "Target/Syntax.h"
#include "Target/CFG.h"
#include "Target/Liveness.h"
#include "Common/Seq.h"
#include "Support/basics.h"

namespace V3DLib {

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
	std::string last_comment;
  for (int i = 0, j = 0; i < instrs.size(); i++) {
    auto &instr = instrs[i];

    if (instr.is_label()) {
      labels[instr.label()] = j;

			// Grumbl why doesnt 'cmt << work' here?
			if (!last_comment.empty()) {
				last_comment += "; ";
			}

			last_comment += "Label L";
			last_comment += std::to_string(instr.label());
    } else {
      newInstrs << instr;

     	newInstrs.back().comment(last_comment);
			last_comment.clear();

      j++;
    }
  }
	assert(last_comment.empty());

  // Second, convert branch-label instructions.
  for (int i = 0; i < newInstrs.size(); i++) {
    auto &instr = newInstrs[i];

    if (instr.is_branch_label()) {
			Label label = instr.branch_label();
      assert(0 <= label && label < numLabels);

			// Convert branch-to-label to branch-to-target
      int dest = labels[label];
      assert(dest >= 0);

			instr.label_to_target(dest - i);  // pass in offset to label from current instruction

			std::string cmt;
			cmt += "Jump to label L";
			cmt += std::to_string(label);
			instr.comment(cmt);
    }
  }

	instrs = newInstrs;

  delete [] labels;
}

}  // namespace V3DLib

#endif  // _V3DLIB_REMOVELABELS_H_
