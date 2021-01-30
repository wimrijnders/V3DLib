#ifndef _V3DLIB_COMMON_STACK_H_
#define _V3DLIB_COMMON_STACK_H_
#include <memory>
#include <functional>
#include <utility>
#include "Support/debug.h"

namespace V3DLib {

/**
 * TODO: Perhaps replace this with `Seq` as underlying base class
 */
template <class T> class Stack {
  using Ptr = std::shared_ptr<T>;

protected:

  class StackItem {
  public:
    Ptr head;
    StackItem *tail = nullptr;
  };

public:
  ~Stack() { clear(); }

  bool     empty() const { return m_size == 0; }
  unsigned size()  const { return m_size; }

  void push(Ptr x) {
    StackItem *nextTop = new StackItem;
    nextTop->head = x;
    nextTop->tail = m_topItem;
    
    m_topItem = nextTop;
    m_size++;
  }

  Ptr pop() {
    assert(m_size > 0);
    assert(m_topItem != nullptr);
    StackItem *oldTop = m_topItem;

    m_topItem = oldTop->tail;
    Ptr ret   = oldTop->head;

    oldTop->tail = nullptr;
    delete oldTop;

    m_size--;
    return ret;
  }

  Ptr top() const {
    return top_item()->head;
  }

  void clear() {
    while (!empty()) {
      pop();
    }

    assert(m_size == 0);
  }

protected:
  using Callback = std::function<void(T const &)>;

  void each(Callback f) const {
    StackItem *cur = m_topItem;

    while (cur != nullptr) {
      f(*(cur->head));
      cur = cur->tail; 
    }
  }


  StackItem *top_item() const {
    assert(m_size > 0);
    assert(m_topItem != nullptr);
    assert(m_topItem->head.get() != nullptr);
    return m_topItem;
  }

private:
  unsigned int m_size  = 0;
  StackItem *m_topItem = nullptr;
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_STACK_H_
