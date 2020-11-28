#include "StmtStack.h"


namespace V3DLib {

namespace {
	StmtStack *p_stmtStack = nullptr;
} // anon namespace


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


StmtStack &stmtStack() {
	assert(p_stmtStack != nullptr);
	return *p_stmtStack;
}


void clearStack() {
	assert(p_stmtStack != nullptr);
	p_stmtStack = nullptr;
}


void setStack(StmtStack &stmtStack) {
	assert(p_stmtStack == nullptr);
	p_stmtStack = &stmtStack;
}

}  // namespace V3DLib
