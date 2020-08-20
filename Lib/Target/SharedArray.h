////////////////////////////////////////////////////////////////////////////////
// Emulator version of SharedArray
////////////////////////////////////////////////////////////////////////////////
#ifndef _QPULIB_EMULATOR_SHAREDARRAY_H_
#define _QPULIB_EMULATOR_SHAREDARRAY_H_
#include <cassert>
#include <stdio.h>
#include "BufferObject.h"

namespace QPULib {
namespace Target {

// ============================================================================
// Emulation mode
// ============================================================================


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

		address = emu::emuHeap.alloc(n);  // Will abort if no space for allocation
		m_size = n;
  }

  SharedArray() {}

  SharedArray(uint32_t n) : m_size(n) {
		assert(n >= 0);
		alloc(n);
  }

	void setType(BufferType buftype) {
		assert(BufferType::HeapBuffer == emu::emuHeap.buftype);
	}

	/**
	 * Return address in bytes
	 */
  uint32_t getAddress() {
    return address*4;
  }

  T* getPointer() {
    return (T*) &emu::emuHeap[address];
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

		return (T&) emu::emuHeap[address+i];
  }
};


}  // namespace Target
}  // namespace QPULib



#endif  // _QPULIB_EMULATOR_SHAREDARRAY_H_
