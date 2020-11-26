#include "StmtStack.h"


namespace V3DLib {

/**
 * Add passed statement to the end of the current instructions
 *
 * This is a logical operation;
 * a new sequence item is added on top, with the previous tree as the left branch,
 * and the new item as the right branch.
 *
 * The net effect is that the passed instruction is added to the end of the sequence
 * of statements to be compiled.
 */
void StmtStack::append(Stmt *stmt) {
	assert(stmt != nullptr);
	assert(!empty());
	push(mkSeq(apop(), stmt));
}

}  // namespace V3DLib
