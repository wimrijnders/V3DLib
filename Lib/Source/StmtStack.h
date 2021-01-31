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
  using Ptr = std::shared_ptr<StmtStack>;

  void reset();
  void append(Stmt::Ptr stmt);

  StmtStack &operator<<(Stmt::Ptr stmt) {
    append(stmt);
    return *this;
  }

  std::string dump() const;
  bool add_prefetch(Pointer &exp);
  bool add_prefetch(PointerExpr const &exp);
  Stmt *first_in_seq() const;

private:
  Stmt::Ptr prefetch = nullptr;
  int prefetch_count = 0;

  bool init_prefetch();
  void post_prefetch(Ptr assign);
};


StmtStack &stmtStack();
void clearStack();
void initStack(StmtStack &stmtStack);

inline bool add_prefetch(PointerExpr addr) {
  return stmtStack().add_prefetch(addr);
}

inline bool add_prefetch(Pointer addr) {
  return stmtStack().add_prefetch(addr);
}

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_STMTSTACK_H_
