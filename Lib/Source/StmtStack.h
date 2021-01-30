#ifndef _V3DLIB_SOURCE_STMTSTACK_H_
#define _V3DLIB_SOURCE_STMTSTACK_H_
#include "Common/Stack.h"
#include "Stmt.h"

namespace V3DLib {

/**
 * Strictly speaking, this is a tree, not a stack
 */
class StmtStack : public Stack<Stmt> {
public:
  void append(Stmt::Ptr stmt);

  StmtStack &operator<<(Stmt::Ptr stmt) {
    append(stmt);
    return *this;
  }

  std::string dump() const;

  void add_preload(BaseExpr const &exp);

private:
  Stmt::Ptr preload = nullptr;
};


StmtStack &stmtStack();
void clearStack();
void setStack(StmtStack &stmtStack);

void add_preload(BaseExpr const &exp);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_STMTSTACK_H_
