#ifndef _V3DLIB_COMMON_STACK_H_
#define _V3DLIB_COMMON_STACK_H_
#include <memory>
#include <assert.h>

namespace V3DLib {

template <class T> class SStack {
private:
	class StackItem {
	public:
		std::unique_ptr<T> head;
		StackItem *tail = nullptr;
	};

public:
    ~SStack() {
      clear();
    }

		bool empty() const { return m_size == 0; }
		unsigned  size() const { return m_size; }

    void push(T* x) {
      StackItem *oldTop = m_topItem;
      m_topItem       = new StackItem;
      m_topItem->head.reset(x);
      m_topItem->tail = oldTop;
      m_size++;
    }

    void ppop() {
      assert(m_size > 0);
      StackItem *oldTop = m_topItem;
      m_topItem = m_topItem->tail;
      delete oldTop;
      m_size--;
    }

		T* apop() {
      assert(m_size > 0);
      StackItem *oldTop = m_topItem;
			T *ret = m_topItem->head.release();
      m_topItem = m_topItem->tail;
      delete oldTop;
      m_size--;
			return ret;
		}

    T* ttop() {
      assert(m_size > 0);
      return m_topItem->head.get();
    }

    // Clear the stack
    void clear() {
      StackItem *p;

			while (m_topItem != nullptr) {
				ppop();
      }

      assert(m_size == 0);
    }

private:
	unsigned int m_size = 0;
	StackItem *m_topItem = nullptr;
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_STACK_H_
