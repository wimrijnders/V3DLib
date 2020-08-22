#ifndef _QPULIB_COMMON_SHAREDARRAY_H_
#define _QPULIB_COMMON_SHAREDARRAY_H_
#include "BufferObject.h"
#include "../Support/debug.h"
#include "../Support/Platform.h"  // has_vc4

namespace QPULib {

// Identifier for creating a SharedArray view for an entire buffer object
enum HeapView {
	use_as_heap_view
};


/**
 * Reserve and access a memory range in the underlying buffer object.
 *
 * All SharedArray instances use the same global BufferObject (BO) instance.
 * This is how the memory and v3d access already worked.
 *
 * For vc4, this is a change. Previously, each SharedArray instance had its
 * own BO. Experience will tell if this new setup works
 */
template <typename T>
class SharedArray {
public:
	SharedArray() : m_heap(getBufferObject()) {}
  SharedArray(uint32_t n) : m_heap(getBufferObject()) { alloc(n); }

	SharedArray(HeapView do_heap_view) : m_heap(getBufferObject()) {
		assert(do_heap_view == HeapView::use_as_heap_view);
		m_is_heap_view = true;
		m_size = m_heap.size();
		assert(m_size > 0);
		m_usraddr = m_heap.usr_address();
		m_phyaddr = m_heap.phy_address();
	}


  uint32_t getAddress() { return m_phyaddr; }
	uint32_t size() const { return m_size; }


	/**
	 * Get starting address of the section in question
	 *
	 * Needed for vc4, and for v3d in emulator and interpreter mode.
	 */
	T *getPointer() {
		if (Platform::instance().has_vc4) {
  		return (T *) m_phyaddr;
		} else {
    	return (T *) m_usraddr;
		}
	}


	/**
	 * @param n number of 4-byte elements to allocate (so NOT memory size!)
	 */
	void alloc(uint32_t n) {
		assert(n > 0);
		if (m_size != 0) {
			breakpoint
		}
		assert(m_size == 0);
		assert(m_usraddr == nullptr);
		assert(m_phyaddr == 0);
		assert(!m_is_heap_view);

		m_phyaddr = m_heap.alloc_array(sizeof(T)*n, m_usraddr);
		assert(m_usraddr != nullptr);
		//assert(m_phyaddr > 0);  // Can be 0 for emu
		m_size = n;
	}


	/**
	 * Just forget the allocation and size.
	 *
	 * Note that the array is NOT deallocated in the heap.
	 */
	void dealloc() {
		if (m_size > 0) {
			m_phyaddr = 0;
			m_size = 0;
			m_usraddr = nullptr;
		}
	}


  // Subscript
  inline const T operator[] (int i) const {
		assert(i >= 0);
		assert(m_size > 0);
		assert(i < m_size);
		assert(m_usraddr != nullptr);

    T* base = (T *) m_usraddr;
    return (T) base[i];
  }


  // Subscript
  inline T& operator[] (int i) {
		if (i < 0) {
			breakpoint
		}
		assert(i >= 0);
		assert(m_size > 0);
		if (i >= m_size) {
			breakpoint  // Check if this ever happens
		}
		assert(i < m_size);
		assert(m_usraddr != nullptr);

    T* base = (T *) m_usraddr;
    return (T&) base[i];
  }


	/**
	 * Subscript for access using physical address.
	 *
	 * Needed for interpreter and emulator.
	 * Intended for use with T = uint32_t, but you never know.
	 */
  inline T& phy(uint32_t i) {
		//breakpoint
		assert(m_phyaddr % sizeof(T) == 0);
		int index = i - m_phyaddr/sizeof(T);

		return (*this)[index];
  }

private:
	BufferObject &m_heap;
	uint8_t *m_usraddr = nullptr;  // Start of the heap in main memory, as seen by the CPU
	uint32_t m_phyaddr = 0;        // Starting index of memory in GPU space
	uint32_t m_size = 0;           // Number of contained elements (not memory size!)
	bool     m_is_heap_view = false;
};

}  // namespace QPULib

#endif  // _QPULIB_COMMON_SHAREDARRAY_H_
