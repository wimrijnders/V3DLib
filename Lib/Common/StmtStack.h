#ifndef _V3DLIB_COMMON_STMTSTACK_H_
#define _V3DLIB_COMMON_STMTSTACK_H_
#include "Stack.h"
#include "Source/Stmt.h"

namespace V3DLib {

/**
 * Strictly speaking, this is a tree, not a stack
 */
class StmtStack : public SStack<Stmt> {
public:
	void append(Stmt *stmt);
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_STMTSTACK_H_
