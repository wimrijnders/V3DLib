#ifndef _V3DLIB_SOURCE_STMTSTACK_H_
#define _V3DLIB_SOURCE_STMTSTACK_H_
#include <functional>
#include <vector>
#include <map>
#include "Common/Stack.h"
#include "Stmt.h"

namespace V3DLib {


/**
 * 
 */
class StmtStack : public Stack<Stmts> {
  using Parent = Stack<Stmts>;

public:
  using Ptr = std::shared_ptr<StmtStack>;

  void push();
  void push(Stmt::Ptr s);
  Stmt::Ptr last_stmt();

  void reset();
  void append(Stmt::Ptr stmt);
  void append(Stmts const &stmts);

  StmtStack &operator<<(Stmt::Ptr stmt) { append(stmt); return *this; }
  StmtStack &operator<<(Stmts const &stmts)  { append(stmts); return *this; }

  std::string dump() const;

  Stmt *first_in_seq() const;

  void first_prefetch(int prefetch_label);
  void add_prefetch(Pointer &exp, int prefetch_label);
  void add_prefetch(PointerExpr const &exp, int prefetch_label);
  void resolve_prefetches();

private:
  class PrefetchContext {
  public:
    void resolve_prefetches();
    void add_prefetch_label(Stmt::Ptr pre);
    bool tags_empty() const { return m_prefetch_tags.empty(); }
    void post_prefetch(Ptr assign);

  private:
    int prefetch_count = 0;

    Stmts              m_prefetch_tags;
    std::vector<Ptr>   m_assigns;

    bool empty() const { return m_prefetch_tags.empty() &&  m_assigns.empty(); }
  };

  std::map<int, PrefetchContext> prefetches;

  void add_prefetch_label(int prefetch_label);
};


StmtStack &stmtStack();
void clearStack();
void initStack(StmtStack &stmtStack);


inline void prefetch(int prefetch_label = 0) {
  stmtStack().first_prefetch(prefetch_label);
}


template<typename T>
void prefetch(T &dst, PointerExpr src, int prefetch_label = 0) {
  stmtStack().first_prefetch(prefetch_label);
  receive(dst);
  stmtStack().add_prefetch(src, prefetch_label);
}


template<typename T>
void prefetch(T &dst, Pointer &src, int prefetch_label = 0) {
  stmtStack().first_prefetch(prefetch_label);
  receive(dst);
  stmtStack().add_prefetch(src, prefetch_label);
}


using StackCallback = std::function<void()>;
Stmts tempStmt(StackCallback f);

}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_STMTSTACK_H_
