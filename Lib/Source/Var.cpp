#include "Var.h"
#include "Support/debug.h"

namespace V3DLib {

static int globalVarId = 0;  // Used for fresh variable generation


/**
 * Obtain a fresh variable
 * @return a fresh standard variable
 */
Var freshVar() {
	return Var(STANDARD, globalVarId++);
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

}  // namespace V3DLib
