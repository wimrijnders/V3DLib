#ifndef _V3DLIB_COMMON_SHAREDARRAY_H_
#define _V3DLIB_COMMON_SHAREDARRAY_H_
#include <vector>
#include "BufferObject.h"
#include "../Support/debug.h"
#include "../Support/Platform.h"  // has_vc4

namespace V3DLib {

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
	SharedArray() {}
  SharedArray(uint32_t n) { alloc(n); }
  SharedArray(uint32_t n, BufferObject &heap) : m_heap(&heap) { alloc(n); }
	~SharedArray() { dealloc(); }

	void heap_view(BufferObject &heap) {
		assert(!allocated());
		assert(m_heap == nullptr);

		m_heap = &heap;
		m_is_heap_view = true;
		m_size = m_heap->size();
		assert(m_size > 0);
		m_usraddr = m_heap->usr_address();
		m_phyaddr = m_heap->phy_address();
	}


  uint32_t getAddress() { return m_phyaddr; }
	uint32_t size() const { return m_size; }

	void fill(T val) {
  	for (int i = 0; i < (int) size(); i++)
	    (*this)[i] = val;
	}

	/**
	 * Get starting address of the section in question
	 *
	 * Needed for vc4, emulator and interpreter mode.
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
		assert(!allocated());
		assert(n > 0);

		if (m_heap == nullptr) {
			m_heap = &getBufferObject();
		}

		m_phyaddr = m_heap->alloc_array(sizeof(T)*n, m_usraddr);
		m_size = n;
		assert(allocated());
	}


	bool allocated() const {
		if (m_size > 0) {
			assert(m_heap != nullptr);
			// assert(m_phyaddr > 0);  // Can be 0 for emu
			assert(m_usraddr != nullptr);
			return true;
		} else {
			assert(m_phyaddr == 0);
			assert(m_usraddr == nullptr);
			assert(!m_is_heap_view);
			return false;
		}
	}


	/**
	 * Forget the allocation and size and notify the underlying heap.
	 */
	void dealloc() {
		if (m_size > 0) {
			assert(allocated());
			assert(m_heap != nullptr);
			if (!m_is_heap_view) { 
				m_heap->dealloc_array(m_phyaddr, sizeof(T)*m_size);
			}
			m_phyaddr = 0;
			m_size = 0;
			m_usraddr = nullptr;
			m_is_heap_view = false;
		} else {
			assert(!allocated());
		}
	}


  // Subscript
  inline const T operator[] (int i) const {
		assert(allocated());
		assert(i >= 0);
		assert(i < (int) m_size);

    T* base = (T *) m_usraddr;
    return (T) base[i];
  }


  // Subscript
  inline T& operator[] (int i) {
		assert(allocated());
		assertq(i >= 0 && i < (int) m_size, "SharedArray::[]: index outside of possible range", true);

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


	void copyFrom(T const *src, uint32_t size) {
		assert(src != nullptr);
		assert(size <= m_size);

		// TODO: consider using memcpy() instead
		for (uint32_t offset = 0; offset < size; ++offset) {
			(*this)[offset] = src[offset];
		}
	}

	void copyFrom(std::vector<T> const &src) {
		assert(!src.empty());
		assert(src.size() <= m_size);

		// TODO: consider using memcpy() instead
		for (uint32_t offset = 0; offset < src.size(); ++offset) {
			(*this)[offset] = src[offset];
		}
	}


	/**
	 * Debug method for showing a range in a shared array
	 */
	void dump(int first_offset, int last_offset) {
		assert(first_offset >= 0);
		assert(first_offset <= last_offset);
		assert(last_offset < size());

		char const *format = "%8d: 0x%x - %d\n";

		for (int offset = first_offset; offset <= last_offset; ++offset) {
			printf(format, offset, (*this)[offset], (*this)[offset]);
		}

		printf("\n");
	}


private:
	BufferObject *m_heap = nullptr;  // Reference to used heap
	uint8_t *m_usraddr   = nullptr;  // Start of the heap in main memory, as seen by the CPU
	uint32_t m_phyaddr   = 0;        // Starting index of memory in GPU space
	uint32_t m_size      = 0;        // Number of contained elements (not memory size!)
	bool     m_is_heap_view = false;
};

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_SHAREDARRAY_H_
