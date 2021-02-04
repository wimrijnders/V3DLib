#ifndef _V3DLIB_SOURCE_STMTSTACK_H_
#define _V3DLIB_SOURCE_STMTSTACK_H_
#include <functional>
#include <vector>
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

  void first_prefetch();
  void add_prefetch(Pointer &exp);
  void add_prefetch(PointerExpr const &exp);
  Stmt *first_in_seq() const;
  bool prefetch_empty() const { return m_prefetch_tags.empty() &&  m_assigns.empty(); }
  void resolve_prefetches();

private:
  int prefetch_count = 0;

  std::vector<Stmt::Ptr> m_prefetch_tags;
  std::vector<Ptr>       m_assigns;

  void init_prefetch();
  void post_prefetch(Ptr assign);
};


StmtStack &stmtStack();
void clearStack();
void initStack(StmtStack &stmtStack);

template<typename T>
void prefetch(T &dst, PointerExpr src) {
  stmtStack().first_prefetch();
  receive(dst);
  stmtStack().add_prefetch(src);
}

template<typename T>
void prefetch(T &dst, Pointer &src) {
  stmtStack().first_prefetch();
  receive(dst);
  stmtStack().add_prefetch(src);
}

using StackCallback = std::function<void()>;
Stmt::Ptr tempStmt(StackCallback f);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_STMTSTACK_H_
