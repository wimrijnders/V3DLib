//
// Sequence data type
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _V3DLIB_COMMON_SEQ_H_
#define _V3DLIB_COMMON_SEQ_H_
#include <stdlib.h>
#include <string>
#include "Support/debug.h"

namespace V3DLib {

template <class T>
class Seq {
public:
  int const INITIAL_MAX_ELEMS = 1024;

  Seq() { setCapacity(INITIAL_MAX_ELEMS); }
  Seq(int initialSize) { setCapacity(initialSize); }
  Seq(Seq<T> const &seq) { *this = seq; }


  /**
   * Assignment operator - really needed! Default assignment does shallow copy
   */
  Seq<T> & operator=(Seq<T> const &seq) {
    setCapacity(seq.maxElems);

    numElems = seq.numElems;
    for (int i = 0; i < seq.numElems; i++)
      elems[i] = seq.elems[i];

    return *this;
  }
    

  ~Seq() {
    if (elems == nullptr) {  // WRI debug
      breakpoint
    }

    delete [] elems;
    elems = nullptr;
  }


  /**
   * Here's one for the Hall of Shame, previous definition:
   *
   *    bool size() const { return numElems; }
   *
   * How much of an idiot can I be?
   * I kept reading over it for ages, because how can size be wrong, right?
   * Hours of confusion have now been explained.
   */
  int size() const { return numElems; }


  void set_size(int new_size) {
    assertq(new_size > 0, "Seq::set_size(): can not set size to zero");
    setCapacity(new_size);
    numElems = new_size;
  }

  T &get(int index) {
    assertq(!empty(), "seq[]: can not access elements, sequence is empty", true);
    assertq(0 <= index && index < numElems, "Seq[]: index out of range", true);
    return elems[index];
  }


  T operator[](int index) const {
    assertq(!empty(), "seq[]: can not access elements, sequence is empty", true);
    assertq(0 <= index && index < numElems, "Seq[]: index out of range", true);
    return elems[index];
  }


  bool empty() const       { return size() == 0; }
  T &operator[](int index) { return get(index); }
  T &front()               { return get(0); }
  T &back()                { return get(size() - 1); }
  T const &back() const    { return get(size() - 1); }
  T *data()                { return elems; }


  /**
   * Ensure that there is enough capacity in the sequence to contain the
   * given number of elements
   *
   * Resize if required.
   *
   * @param n  requested size of sequence
   */
  void setCapacity(int n) {
    assertq(n > 0, "Seq::setCapacity(): can not set sequence capacity to zero", true);
    if (n <= maxElems) {
      assertq(elems != nullptr, "Seq::setCapacity(): internal storage not initialized");
      return;  // Don't bother resizing if already big enough
    }

    maxElems = n;
    T* newElems = new T[maxElems];

    if (elems != nullptr) {
      for (int i = 0; i < numElems; i++) {
        newElems[i] = elems[i];
      }
    }

    delete [] elems;
    elems = newElems;
  }


  void append(T x) {
    extend_by(1);
    numElems++;
    elems[numElems-1] = x;
  }


  void clear()      { numElems = 0; }
  void deleteLast() { numElems--; }
  void push(T x)    { append(x); }


  T pop() {
    assertq(numElems > 0, "Seq::pop(): sequence is empty, nothing to return");
    numElems--;
    return elems[numElems];
  }


  /**
   * Insert item at specified location
   */
  void insert(int index, T const &item) {
    shift_tail(index, 1);
    elems[index] = item;
  }


  /**
   * Insert passed sequence at specified location
   */
  void insert(int index, Seq<T> const &items) {
    shift_tail(index, items.size());

    for (int j = 0; j < items.size(); j++) {
      elems[index + j] = items.elems[j];
    }
  }


  /**
   * Remove element at index
   */
  T remove(int index) {
    assertq(numElems > 0, "Seq::remove(): sequence is empty, nothing to remove");
    assertq(0 <= index && index < numElems, "Seq::remove(): index out of range");
    T x = elems[index];

    for (int j = index; j < numElems-1; j++) {
      elems[j] = elems[j+1];
    }

    numElems--;
    return x;
  }


  Seq<T> &operator<<(T const &rhs) { 
    append(rhs);
    return *this;
  }


  Seq<T> &operator<<(Seq<T> const &rhs) { 
    for (int j = 0; j < rhs.size(); j++) {
      append(rhs.elems[j]);
    }

    return *this;
  }


private:
  int maxElems = 0;
  int numElems = 0;
  T* elems     = nullptr;


  /**
   * Shift tail of sequence n positions, starting from index
   *
   * `numElems` gets adjusted here; this means that there are some unassigned
   * positions in the sequence that still need to be filled in by caller.
   */
  void shift_tail(int index, int n ) {
    assertq(n > 0, "Seq::shift_tail(): can not shift zero length");
    assertq(index >= 0 && index <= size(), "Seq::shift_tail(): index out of range");  // index == size allowed, amounts to append

    int prevNum = numElems;
     extend_by(n);
    numElems += n;

    if (index < size()) {  // for index == size, nothing to move
      for (int i = prevNum - 1; i >= index; --i) {
        elems[i + n] = elems[i];
      }
    }
  }


  /**
   * Ensure that sequence can contain current num elements + passed value
   *
   */
  void extend_by(int step = 1) {
    assertq(step > 0, "Seq::extend_by(): can not extend with zero length");

    int newSize = maxElems;
    if (newSize == 0) {
      assertq(elems == nullptr, "Seq::extend_by(): sequence has no internal storage");
      newSize = INITIAL_MAX_ELEMS;

      // Following actually happened during gdb debug sessions, prob due to skipping of ctor Set.
      assertq(newSize != 0, "Seq::extend_by(): Weirdness! newSize assigned but is zero anyway.");
    }

    while (newSize < (numElems + step))
      newSize *= 2;

    setCapacity(newSize);
  }

};


using IntList  = Seq<int32_t>;
using UIntList = Seq<uint32_t>;

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_SEQ_H_
