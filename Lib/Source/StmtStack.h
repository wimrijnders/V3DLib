#ifndef _V3DLIB_SOURCE_STMTSTACK_H_
#define _V3DLIB_SOURCE_STMTSTACK_H_
#include <functional>
#include "Common/Stack.h"
#include "Source/Ptr.h"
#include "Stmt.h"

namespace V3DLib {

/**
 * 
 */
class StmtStack : public Stack<Stmt> {
public:
  void reset();
  void append(Stmt::Ptr stmt);

  StmtStack &operator<<(Stmt::Ptr stmt) {
    append(stmt);
    return *this;
  }

  std::string dump() const;

  void add_preload(V3DLib::Ptr<Int> &src);
  void add_preload(BaseExpr const &exp);
  Stmt *first_in_seq() const;

private:
  Stmt::Ptr preload = nullptr;
};


StmtStack &stmtStack();
void clearStack();
void initStack(StmtStack &stmtStack);


template <typename T>
inline void add_preload(PtrExpr<T> addr) {
  stmtStack().add_preload(addr);
}


template <typename T>
inline void add_preload(Ptr<T> &addr) {
  stmtStack().add_preload(addr);
}


using StackCallback = std::function<void()>;
using StackPtr = std::shared_ptr<StmtStack>;

StackPtr tempStack(StackCallback f);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_STMTSTACK_H_
