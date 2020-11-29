#ifndef _V3DLIB_COMMON_STACK_H_
#define _V3DLIB_COMMON_STACK_H_
#include <memory>
#include "Support/debug.h"

namespace V3DLib {

template <class T> class Stack {
private:
	class StackItem {
	public:
		std::unique_ptr<T> head;
		std::unique_ptr<StackItem> tail;
	};

public:
	Stack() {}

	bool     empty() const { return m_size == 0; }
	unsigned size()  const { return m_size; }

	void push(T* x) {
		std::unique_ptr<StackItem> nextTop;
		nextTop.reset(new StackItem);
		nextTop->head.reset(x);
		nextTop->tail.swap(m_topItem);

		m_topItem.swap(nextTop);
		m_size++;
	}

		T* pop() {
      assert(m_size > 0);
			T *ret = m_topItem->head.release();
			m_topItem.reset(m_topItem->tail.release());
      m_size--;
			return ret;
		}

    T* top() const {
      assert(m_size > 0);
      return m_topItem->head.get();
    }

    // Clear the stack
    void clear() {
			while (m_topItem.get() != nullptr) {
				breakpoint
				ppop();
      }

      assert(m_size == 0);
    }


private:
	unsigned int m_size = 0;
	std::unique_ptr<StackItem> m_topItem;

	void ppop() {
		assert(m_size > 0);
		m_topItem.reset(m_topItem->tail.release());
		m_size--;
	}
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_STACK_H_
