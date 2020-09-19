#ifndef _QPULIB_STACK_H_
#define _QPULIB_STACK_H_
#include <stdlib.h>
#include <assert.h>

namespace QPULib {

template <class T> class StackItem {
public:
    T* head;
    StackItem<T>* tail;
};


template <class T> class Stack {
public:
    unsigned int size;
    StackItem<T>* topItem;

    Stack() {
      topItem = NULL;
      size    = 0;
    }

    ~Stack() {
      clear();
    }

    void push(T* x) {
      StackItem<T>* oldTop = topItem;
      topItem       = new StackItem<T>;
      topItem->head = x;
      topItem->tail = oldTop;
      size++;
    }

    void pop() {
      assert(size > 0);
      StackItem<T>* oldTop = topItem;
      topItem = topItem->tail;
      delete oldTop;
      size--;
    }

    T* top() {
      assert(size > 0);
      return topItem->head;
    }

    // Replace the top element
    void replace(T* x) {
      topItem->head = x;
    }

    // Clear the stack
    void clear() {
      StackItem<T>* p;
      for (int i = 0; i < size; i++) {
        p = topItem->tail;
        delete topItem;
        topItem = p;
      }
      size = 0;
    }
};

}  // namespace QPULib

#endif  // _QPULIB_STACK_H_
