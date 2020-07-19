////////////////////////////////////////////////////////////////////////////////
// Emulator version of SharedArray
////////////////////////////////////////////////////////////////////////////////
#ifndef _QPULIB_EMULATOR_SHAREDARRAY_H_
#define _QPULIB_EMULATOR_SHAREDARRAY_H_
#include <cassert>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>     /* abort, NULL */


extern uint32_t emuHeapEnd;
extern int32_t* emuHeap;


namespace QPULib {
namespace Target {

// ============================================================================
// Emulation mode
// ============================================================================

// When in EMULATION_MODE allocate memory from a pre-allocated pool.
#define EMULATOR_HEAP_SIZE 1024*65536


// Implementation
template <typename T> class SharedArray {
 private:
	// Disallow assignment
	void operator=(SharedArray<T> a);
	void operator=(SharedArray<T>& a);

  uint32_t m_size = 0;

 public:

  uint32_t address;
  uint32_t size() { return m_size; }

  // Allocation
  void alloc(uint32_t n) {
    //printf("Allocating %d\n", n);
		if (n == 0) {
			assert(m_size == 0);
			return;
		}

    if (emuHeap == NULL) {
      emuHeapEnd = 0;
      emuHeap = new int32_t [EMULATOR_HEAP_SIZE];
    }
    if (emuHeapEnd+n >= EMULATOR_HEAP_SIZE) {
      printf("QPULib: heap overflow (increase EMULATOR_HEAP_SIZE)\n");
      abort();
    }
    else {
      address = emuHeapEnd;
      emuHeapEnd += n;
      m_size = n;
    }
  }

  SharedArray() {}

  SharedArray(uint32_t n) : m_size(n) {
		assert(n >= 0);
		alloc(n);
  }

  uint32_t getAddress() {
    return address*4;
  }

  T* getPointer() {
    return (T*) &emuHeap[address];
  }


	/**
   * Deallocation (does nothing in emulation mode)
	 *
	 * Abandon the data we are referring to.
	 * The data still remains on the heap (no method to remove it)
	 */ 
  void dealloc() {
		m_size = 0;
	}

  // Subscript
  T& operator[] (int i) {
		assert(i >= 0);
		assert(m_size > 0);
		if (i >= m_size) {
			printf("i: %d, size: %d\n", i, m_size);
		}
		assert(i < m_size);

    if (address+i >= EMULATOR_HEAP_SIZE) {
      printf("QPULib: accessing off end of heap\n");
      exit(EXIT_FAILURE);
    }
    else
      return (T&) emuHeap[address+i];
  }
};


}  // namespace Target
}  // namespace QPULib



#endif  // _QPULIB_EMULATOR_SHAREDARRAY_H_
