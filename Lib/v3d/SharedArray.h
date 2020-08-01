#ifndef _QPULIB_V3D_SHAREDARRAY_H_
#define _QPULIB_V3D_SHAREDARRAY_H_
#include <cassert>
#include <stdint.h>
#include "../debug.h"


namespace QPULib {
namespace v3d {

class SharedArrayBase;


template<typename T>
class ArrayView {
public:
	ArrayView(char *base, uint32_t size) : m_base(base), m_size(size/sizeof(T)) {
		assert(base != nullptr);
		assert(size > 0);
	}


	uint32_t size() const { return m_size; }
  uint32_t getAddress() const { return  (uint32_t) m_base; }

	void copyFrom(T const *src, uint32_t size) {
		assert(src != nullptr);
		assert(size < m_size);

		// TODO: consider using memcpy() instead
		for (uint32_t offset = 0; offset < size; ++offset) {
			(*this)[offset] = offset;
		}
	}

  // Subscript
  inline const T operator[] (int i) const {
		assert(i >= 0);
		assert(m_size > 0);
		assert(i < m_size);

    T* base = (T *) m_base;
    return (T) base[i];
  }


  inline T& operator[] (int i) {
		assert(i >= 0);
		assert(m_size > 0);
		assert(i < m_size);

    T* base = (T *) m_base;
    return (T&) base[i];
  }

private:
  char *   m_base  = nullptr;
	uint32_t m_size  = 0;        // size array in element types
};


class SharedArrayBase {
public:
	SharedArrayBase() {} 
	~SharedArrayBase(); 

  uint32_t getAddress() const { return  (uint32_t) usraddr; }
  uint32_t getPhyAddr() const { return  (uint32_t) phyaddr; }
  uint32_t getHandle()  const { return  (uint32_t) handle; }

//	void copyFrom(SharedArrayBase const &src, uint32_t offset);

	template<typename T>
	ArrayView<T> alloc_view(uint32_t size_in_bytes) {
		assert(m_mem_size > 0);
		assert(m_offset + size_in_bytes < m_mem_size);
		uint32_t prev_offset = m_offset;
		m_offset += size_in_bytes;

		return ArrayView<T>( ((char *) usraddr) + prev_offset, size_in_bytes);
	}

protected:
  void *   usraddr    = nullptr;
	uint32_t m_offset   = 0;
  uint32_t m_mem_size = 0;  // Memory size in bytes

  void alloc_mem(uint32_t n);
	void dealloc_mem();

private:
  uint32_t handle  = 0;
	uint32_t phyaddr = 0;

};


template <typename T> class SharedArray : public SharedArrayBase {
public:
	SharedArray() {} 	SharedArray(uint32_t n) { alloc(n); } 
	~SharedArray() { dealloc(); } 

	uint32_t size() const { return m_size; }

	/**
	 * @param n number of 4-byte elements to allocate (so NOT memory size!)
	 */
	void alloc(uint32_t n) {
		assert(m_size == 0);
		alloc_mem(sizeof(T)*n);
		m_size = n;
	}


	void dealloc() {
		if (m_size > 0) {
			dealloc_mem();
			m_size = 0;
		}
	}


  // Subscript
  inline T& operator[] (int i) {
		assert(i >= 0);
		assert(m_size > 0);
		assert(i < m_size);

    T* base = (T *) usraddr;
    return (T&) base[i];
  }

private:
  // Disallow assignment & copying
  void operator=(SharedArray<T> a);
  void operator=(SharedArray<T>& a);
  SharedArray(const SharedArray<T>& a);

	uint32_t m_size = 0;  // Number of contained elements (not memory size!)
};


}  // namespace v3d
}  // namespace QPULib

#endif  // _QPULIB_V3D_SHAREDARRAY_H_
