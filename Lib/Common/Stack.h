#ifndef _V3DLIB_COMMON_STACK_H_
#define _V3DLIB_COMMON_STACK_H_
//#include <stdlib.h>
#include <assert.h>

namespace V3DLib {

template <class T> class SStack {
private:
	class StackItem {
	public:
		T* head;
		StackItem *tail;
	};

public:
    SStack() {
      m_topItem = nullptr;
      m_size    = 0;
    }

    ~SStack() {
      clear();
    }

		bool empty() const { return m_size == 0; }
		unsigned  size() const { return m_size; }

    void push(T* x) {
      StackItem *oldTop = m_topItem;
      m_topItem       = new StackItem;
      m_topItem->head = x;
      m_topItem->tail = oldTop;
      m_size++;
    }

    void pop() {
      assert(m_size > 0);
      StackItem *oldTop = m_topItem;
      m_topItem = m_topItem->tail;
      delete oldTop;
      m_size--;
    }

    T* top() {
      assert(m_size > 0);
      return m_topItem->head;
    }

    // Replace the top element
    void replace(T* x) {
      m_topItem->head = x;
    }

    // Clear the stack
    void clear() {
      StackItem *p;
      for (int i = 0; i < m_size; i++) {
        p = m_topItem->tail;
        delete m_topItem;
        m_topItem = p;
      }
      m_size = 0;
    }

private:
	unsigned int m_size = 0;
	StackItem *m_topItem = nullptr;
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_STACK_H_
