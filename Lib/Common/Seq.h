//
// Sequence data type
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _QPULIB_SEQ_H_
#define _QPULIB_SEQ_H_
#include <stdlib.h>
#include <string>
#include "Support/debug.h"

#define INITIAL_MAX_ELEMS 1024

namespace QPULib {

template <class T> class Seq {
public:
    int maxElems = 0;
    int numElems = 0;
    T* elems     = nullptr;

    Seq() { init(INITIAL_MAX_ELEMS); }
    Seq(int initialSize) { init(initialSize); }

    // Copy constructor
    Seq(Seq<T> const &seq) {
			*this = seq;
    }

		// Assignment operator - really needed! Other assignment does shallow copy
    Seq<T> & operator=(Seq<T> const &seq) {
      init(seq.maxElems);

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

		T &operator[](int index) {
			assert(0 <= index && index < numElems);
			return elems[index];
		}

    // Set capacity of sequence
    void setCapacity(int n) {
			assert(n > 0);
			if (n <= maxElems) {
				assert(elems != nullptr);
				return;
			}

      maxElems = n;
      T* newElems = new T[maxElems];

      for (int i = 0; i < numElems; i++) {
        newElems[i] = elems[i];
			}

      delete [] elems;
      elems = newElems;
    }

    // Extend size of sequence by one
    void extend() {
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
    void deleteLast() {
      numElems--;
    }

    void push(T x) { append(x); }

    T pop() {
			assert(numElems > 0);
      numElems--;
      return elems[numElems];
    }

    // Clear the sequence
    void clear() { numElems = 0; }

		/**
     * Check if given value already in sequence
		 */
    bool member(T x) {
      for (int i = 0; i < numElems; i++) {
        if (elems[i] == x) return true;
			}
      return false;
    }


		/**
     * Insert element into sequence if not already present
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


    // Remove element at index
    T remove(int index) {
			assert(numElems > 0);
      assert(0 <= index && index < numElems);
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

	void init(int initialSize) {
		//numElems = 0;
    setCapacity(initialSize);
	}


	/**
	 * Shift tail of sequence n positions, starting from index
	 */
	void shift_tail(int index, int n ) {
		assert(index >= 0);
		assert(index < size());
		assert(n > 0);
   	setCapacity(numElems + n);

		for (int i = numElems - 1; i >= index; --i) {
			elems[i + n] = elems[i];
		}

		numElems += n;
	}

};


// A small sequence is just a sequence with a small initial size
template <class T> class SmallSeq : public Seq<T> {
  public:
    SmallSeq() : Seq<T>(8) {};
};

}  // namespace QPULib

#endif  // _QPULIB_SEQ_H_
