#ifndef _QPULIB_SOURCE_VAR_H_
#define _QPULIB_SOURCE_VAR_H_

namespace QPULib {

// ============================================================================
// Variables
// ============================================================================

// What kind of variable is it
enum VarTag {
    STANDARD     // A standard variable that can be stored
                 // in a general-purpose register on a QPU
  , UNIFORM      // (Read-only.)  Reading this variable will consume a value
                 // (replicated 16 times) from the QPU's UNIFORM FIFO
                 // (this is how parameters are passed to kernels).
  , QPU_NUM      // (Read-only.) Reading this variable will yield the
                 // QPU's unique id (replicated 16 times).
  , ELEM_NUM     // (Read-only.) Reading this variable will yield a vector
                 // containing the integers from 0 to 15.
  , VPM_READ     // (Read-only.) Read a vector from the VPM.
  , VPM_WRITE    // (Write-only.) Write a vector to the VPM.
  , TMU0_ADDR    // (Write-only.) Initiate load via TMU

	, DUMMY        // No variable. As a source variable, it indicates that given operation has no input
	               // TODO: As a destination variable, it indicates that the result can be ignored
};

typedef int VarId;

struct Var {
  VarTag tag;

  // A unique identifier for a standard variable
  VarId id;

	bool isUniformPtr = false;
};


Var freshVar();
int getFreshVarCount();
void resetFreshVarGen(int val = 0);

}  // namespace QPULib

#endif  // _QPULIB_SOURCE_VAR_H_
