//
// Sequence data type
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _QPULIB_SEQ_H_
#define _QPULIB_SEQ_H_
#include <stdlib.h>
#include <assert.h>
#include <string>

#define INITIAL_MAX_ELEMS 1024

namespace QPULib {

template <class T> class Seq {
  private:
    // Initialisation
    void init(int initialSize)
    {
      maxElems = initialSize;
      numElems = 0;
      elems    = new T[initialSize];
    }

  public:
    int maxElems;
    int numElems;
    T* elems;

    Seq() { init(INITIAL_MAX_ELEMS); }
    Seq(int initialSize) { init(initialSize); }

    // Copy constructor
    Seq(const Seq<T>& seq) {
      init(seq.maxElems);
      numElems = seq.numElems;
      for (int i = 0; i < seq.numElems; i++)
        elems[i] = seq.elems[i];
    }

    ~Seq() { delete [] elems; }

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

		T &operator[](int index) { return elems[index]; }

    // Set capacity of sequence
    void setCapacity(int n) {
			if (n <= maxElems) {
				return;
			}

      maxElems = n;
      T* newElems = new T[maxElems];
      for (int i = 0; i < numElems-1; i++)
        newElems[i] = elems[i];
      delete [] elems;
      elems = newElems;
    }

    // Extend size of sequence by one
    void extend()
    {
      numElems++;
      if (numElems > maxElems)
        setCapacity(maxElems*2);
    }

    // Append
    void append(T x) {
      extend();
      elems[numElems-1] = x;
    }

    // Delete last element
    void deleteLast()
    {
      numElems--;
    }

    void push(T x) { append(x); }

    T pop() {
      numElems--;
      return elems[numElems];
    }

    // Clear the sequence
    void clear() { numElems = 0; }

    // Is given value already in sequence?
    bool member(T x) {
      for (int i = 0; i < numElems; i++)
        if (elems[i] == x) return true;
      return false;
    }

		/**
     * Insert element into sequence if not already present
		 *
		 * **NOTE:** This is actually a set-method
		 */
    bool insert(T x) {
      bool alreadyPresent = member(x);
      if (!alreadyPresent) append(x);
      return !alreadyPresent;
    }


		/**
		 * Insert item at specified location
		 */
		void insert(int index, T const &item) {
			assert(index >= 0);
			assert(index < size());
    	setCapacity(numElems + 1);

			// Shift tail one position
			for (int i = numElems - 1; i >= index; --i) {
				elems[i + 1] = elems[i];
			}

			// Insert new item
			elems[index] = item;

			numElems++;
		}

    // Remove element at index
    T remove(int index) {
      assert(index < numElems);
      T x = elems[index];
      for (int j = index; j < numElems-1; j++)
        elems[j] = elems[j+1];
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
};

// A small sequence is just a sequence with a small initial size
template <class T> class SmallSeq : public Seq<T> {
  public:
    SmallSeq() : Seq<T>(8) {};
};

}  // namespace QPULib

#endif  // _QPULIB_SEQ_H_
