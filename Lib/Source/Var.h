#ifndef _V3DLIB_SOURCE_VAR_H_
#define _V3DLIB_SOURCE_VAR_H_
#include <string>

namespace V3DLib {

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
	Var(VarTag tag, VarId id = 0) : m_tag(tag), m_id(id) {}

	VarTag tag() const { return m_tag; }
	VarId id() const { return m_id; }
	bool isUniformPtr () const;
	void setUniformPtr();

	std::string disp() const;

private:
  VarTag m_tag;
  VarId  m_id = 0; // A unique identifier for a standard variable
	bool   m_isUniformPtr = false;
};


Var freshVar();
int getFreshVarCount();
void resetFreshVarGen(int val = 0);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_VAR_H_
