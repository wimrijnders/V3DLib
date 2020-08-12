///////////////////////////////////////////////////////////////////////////////
// SharedArray implementation is wrong (currently BufferObject)! Need to redo
///////////////////////////////////////////////////////////////////////////////

#ifndef _QPULIB_V3D_SHAREDARRAY_H_
#define _QPULIB_V3D_SHAREDARRAY_H_
#include "BufferObject.h"


namespace QPULib {
namespace v3d {

template <typename T> class SharedArray : public BufferObject,  public ISharedArray {
public:
	SharedArray() {}
	SharedArray(uint32_t n) { alloc(n); } 
	~SharedArray() { dealloc(); } 

	uint32_t size() const { return m_size; }
  uint32_t getPhyAddr() const override { return BufferObject::getPhyAddr(); }
  uint32_t getHandle()  const override { return BufferObject::getHandle(); }

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
