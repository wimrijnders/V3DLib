//
// Sequence data type
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _QPULIB_SEQ_H_
#define _QPULIB_SEQ_H_
#include <stdlib.h>
#include <string>
#include "Support/debug.h"


namespace QPULib {

template <class T> class Seq {
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


	T *data() { return elems; }


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
		assert(new_size > 0);
		setCapacity(new_size);
		numElems = new_size;
	}

	bool empty() const { return size() == 0; }

		T &get(int index) {
			assertq(0 <= index && index < numElems, "Seq[] index out of range", true);
			return elems[index];
		}

		T &operator[](int index) {
			return get(index);
		}

		T operator[](int index) const {
			assert(0 <= index && index < numElems);
			return elems[index];
		}

		T const &back() const {
			assert(!empty());
			return get(size() - 1);
		}


	/**
	 * Ensure that there is enough capacity in the sequence to contain the
	 * given number of elements
	 *
	 * Resize if required.
	 *
	 * @param n  requested size of sequence
	 */
	void setCapacity(int n) {
		assert(n > 0);
		if (n <= maxElems) {
			assert(elems != nullptr);
			return;  // Don't bother resizing if already big enough
		}

		maxElems = n;
		T* newElems = new T[maxElems];

		if (elems != nullptr) {
			assert(maxElems > 0);
			for (int i = 0; i < numElems; i++) {
				newElems[i] = elems[i];
			}
		}

		delete [] elems;
		elems = newElems;
	}


    // Append
    void append(T x) {
      extend_by(1);
			numElems++;
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
		assert(index >= 0);
		assert(n > 0);
		assert(index <= size());  // index == size amounts to append

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
		assert(step > 0);

		int newSize = maxElems;
		if (newSize == 0) {
			assert(elems == nullptr);
			newSize = INITIAL_MAX_ELEMS;
			assert(newSize != 0);  // Actually happened during gdb debug sessions, prob due to skipping of ctor Set.
		}

		while (newSize < (numElems + step))
			newSize *= 2;

		setCapacity(newSize);
	}

};


/**
 * A small sequence is a sequence with a small initial size
 */
template <class T> class SmallSeq : public Seq<T> {
  public:
    SmallSeq() : Seq<T>(8) {};
};

}  // namespace QPULib

#endif  // _QPULIB_SEQ_H_
