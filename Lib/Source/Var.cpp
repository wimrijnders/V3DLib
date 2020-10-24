#include "Var.h"
#include "Support/debug.h"

namespace QPULib {

static int globalVarId = 0;  // Used for fresh variable generation


/**
 * Obtain a fresh variable
 * @return a fresh standard variable
 */
Var freshVar() {
	Var v;
	v.tag = STANDARD;
	v.id  = globalVarId++;
	return v;
}


/**
 * Returns number of fresh vars used
 */
int getFreshVarCount() {
	return globalVarId;
}


/**
 * Reset fresh variable generator
 */
void resetFreshVarGen(int val) {
	assert(val >= 0);
	globalVarId = val;
}

}  // namespace QPULib
