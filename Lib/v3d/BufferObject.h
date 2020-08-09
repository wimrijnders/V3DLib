#ifndef _QPULIB_V3D_BUFFEROBJECT_H_
#define _QPULIB_V3D_BUFFEROBJECT_H_
#include <cassert>
#include <vector>
#include <stdint.h>
#include "../debug.h"


namespace QPULib {
namespace v3d {


class ISharedArray {
public: 
	virtual ~ISharedArray() {}

  //virtual uint32_t getAddress() const = 0;
  virtual uint32_t getHandle()  const { assert(false); return 0; }
  virtual uint32_t getPhyAddr() const = 0;
};


class BufferObject;


template<typename T>
class ArrayView : public ISharedArray {
public:
	ArrayView(uint8_t *usraddr, uint32_t phyaddr, uint32_t size_in_bytes) :
		m_usraddr(usraddr),
		m_phyaddr(phyaddr),
		m_size(size_in_bytes/sizeof(T)) {
		assert(usraddr != nullptr);
		assert(size_in_bytes > 0);
		assert(phyaddr % 4 == 0);
	}


	uint32_t size() const { return m_size; }
  uint32_t getPhyAddr() const override { return  (uint32_t) m_phyaddr; }

	void copyFrom(T const *src, uint32_t size) {
		assert(src != nullptr);
		assert(size < m_size);

		// TODO: consider using memcpy() instead
		for (uint32_t offset = 0; offset < size; ++offset) {
			(*this)[offset] = src[offset];
		}
	}


	void copyFrom(std::vector<T> const &src) {
		assert(!src.empty());
		assert(src.size() < m_size);

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

    T* base = (T *) m_usraddr;
    return (T) base[i];
  }


  inline T& operator[] (int i) {
		assert(i >= 0);
		assert(m_size > 0);
		assert(i < m_size);

    T* base = (T *) m_usraddr;
    return (T&) base[i];
  }

private:
  uint8_t *m_usraddr  = nullptr;
	uint32_t m_phyaddr = 0;
	uint32_t m_size  = 0;        // size array in element types
};


class BufferObject {
public:
	BufferObject() {} 
	~BufferObject(); 

  uint32_t getAddress() const { return  (uint32_t) usraddr; }
  uint32_t getPhyAddr() const { return  (uint32_t) phyaddr; }
  uint32_t getHandle()  const { return  (uint32_t) handle; }

//	void copyFrom(SharedArrayBase const &src, uint32_t offset);

	template<typename T>
	ArrayView<T> alloc_view(uint32_t size_in_bytes) {
		assert(m_mem_size > 0);
		assert(m_offset + size_in_bytes < m_mem_size);
		assert(size_in_bytes % 4 == 0);
		uint32_t prev_offset = m_offset;
		m_offset += size_in_bytes;

		return ArrayView<T>(usraddr + prev_offset, phyaddr + prev_offset, size_in_bytes);
	}

protected:
  uint8_t *usraddr    = nullptr;
	uint32_t m_offset   = 0;
  uint32_t m_mem_size = 0;  // Memory size in bytes

  void alloc_mem(uint32_t n);
	void dealloc_mem();

private:
  uint32_t handle  = 0;
	uint32_t phyaddr = 0;

};

}  // namespace v3d
}  // namespace QPULib

#endif  // _QPULIB_V3D_BUFFEROBJECT_H_
