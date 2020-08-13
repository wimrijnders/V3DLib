///////////////////////////////////////////////////////////////////////////////
// SharedArray implementation is wrong (currently BufferObject)! Need to redo
///////////////////////////////////////////////////////////////////////////////

#ifndef _QPULIB_V3D_SHAREDARRAY_H_
#define _QPULIB_V3D_SHAREDARRAY_H_
#include "BufferObject.h"


namespace QPULib {
namespace v3d {

template <typename T> class SharedArray {
public:
	SharedArray() : m_heap(getMainHeap()) {}
	SharedArray(uint32_t n) : m_heap(getMainHeap()) { alloc(n); } 
	SharedArray(uint32_t n, BufferObject &heap) : m_heap(heap) { alloc(n); } 
	~SharedArray() { dealloc(); } 

	uint32_t size() const { return m_size; }
  uint32_t getPhyAddr() const { return m_phyaddr; }
  uint32_t getHandle()  const { return m_heap.getHandle(); }

	/**
	 * @param n number of 4-byte elements to allocate (so NOT memory size!)
	 */
	void alloc(uint32_t n) {
		assert(m_size == 0);
		m_phyaddr = m_heap.alloc_array(sizeof(T)*n);
		assert(m_phyaddr > 0);
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
		}
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


  // Subscript
  inline const T operator[] (int i) const {
		assert(i >= 0);
		assert(m_size > 0);
		assert(i < m_size);

    T* base = (T *) m_heap.getAddress();
    return (T) base[i];
  }


  // Subscript
  inline T& operator[] (int i) {
		assert(i >= 0);
		assert(m_size > 0);
		assert(i < m_size);

    T* base = (T *) m_heap.getAddress();
    return (T&) base[i];
  }

private:
  // Disallow assignment & copying
  void operator=(SharedArray<T> a);
  void operator=(SharedArray<T>& a);
  SharedArray(const SharedArray<T>& a);

	uint32_t m_phyaddr = 0;
	uint32_t m_size = 0;  // Number of contained elements (not memory size!)
	BufferObject &m_heap;
};


}  // namespace v3d
}  // namespace QPULib

#endif  // _QPULIB_V3D_SHAREDARRAY_H_
