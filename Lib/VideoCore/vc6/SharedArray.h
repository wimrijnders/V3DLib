#ifndef _QPULIB_V3D_SHAREDARRAY_H_
#define _QPULIB_V3D_SHAREDARRAY_H_
#include <cassert>
#include <stdint.h>
#include "../../debug.h"


namespace QPULib {
namespace v3d {

class SharedArrayBase {
public:
	SharedArrayBase() {} 
	~SharedArrayBase(); 

  uint32_t getAddress() { return  (uint32_t) usraddr; }

protected:
  void* usraddr = nullptr;

  void alloc_mem(uint32_t n);
	void dealloc_mem();

private:
  uint32_t handle = 0;
	uint32_t phyaddr = 0;
  uint32_t m_mem_size = 0;  // Memory size in bytes

};


template <typename T> class SharedArray : public SharedArrayBase {
public:
	SharedArray() {} 
	SharedArray(uint32_t n) { alloc(n); } 
	~SharedArray() { dealloc(); } 

	uint32_t size() { return m_size; }

	/**
	 * @param n number of 4-byte elements to allocate (so NOT memory size!)
	 */
	void alloc(uint32_t n) {
		assert(m_size == 0);
		alloc_mem(sizeof(T)*n);
		m_size = n;
	}


	void dealloc() {
		breakpoint
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
