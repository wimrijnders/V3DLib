#ifndef _V3DLIB_TARGET_INSTR_LABEL_H_
#define _V3DLIB_TARGET_INSTR_LABEL_H_

namespace V3DLib {

// ============================================================================
// Fresh label generation
//
// Labels are used for for branching, and represented by integer identifiers.
// These are translated to actual branch targets in the final assembly phase.
// ============================================================================

typedef int Label;

Label freshLabel();
int getFreshLabelCount();
void resetFreshLabelGen();
void resetFreshLabelGen(int val);

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_INSTR_LABEL_H_
