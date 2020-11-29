#ifndef _V3DLIB_COMMON_STACK_H_
#define _V3DLIB_COMMON_STACK_H_
#include "Support/debug.h"

namespace V3DLib {

/**
 * TODO: Perhaps replace this with `Seq` as underlying base class
 */
template <class T> class Stack {
private:
	class StackItem {
	public:
		T *head         = nullptr;;
		StackItem *tail = nullptr;
	};

public:
	~Stack() { clear(); }

	bool     empty() const { return m_size == 0; }
	unsigned size()  const { return m_size; }

	void push(T* x) {
		StackItem *nextTop = new StackItem;
		nextTop->head = x;
		nextTop->tail = m_topItem;
		
		m_topItem = nextTop;
		m_size++;
	}

	T *pop() {
		assert(m_size > 0);
		assert(m_topItem != nullptr);
		StackItem *oldTop = m_topItem;

		m_topItem = oldTop->tail;
		T *ret    = oldTop->head;

		oldTop->head = nullptr;
		oldTop->tail = nullptr;
		delete oldTop;

		m_size--;
		return ret;
	}

	T *top() const {
		assert(m_size > 0);
		assert(m_topItem != nullptr);
		assert(m_topItem->head != nullptr);
		return m_topItem->head;
	}

	void clear() {
		while (m_topItem != nullptr) {
			T *p = pop();
			delete p;
		}

		assert(m_size == 0);
	}

private:
	unsigned int m_size  = 0;
	StackItem *m_topItem = nullptr;
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_STACK_H_
