#ifndef _QPULIB_V3D_BUFFEROBJECT_H_
#define _QPULIB_V3D_BUFFEROBJECT_H_
#include <cassert>
#include <vector>
#include <stdint.h>
#include "../debug.h"


namespace QPULib {
namespace v3d {


/**
 * This behaves like an array of uint32_t.
 */
class BufferObject {
public:
	BufferObject(uint32_t size) { alloc_mem(size); }
	BufferObject() {} 
	~BufferObject(); 

  uint32_t getAddress() const { return  (uint32_t) usraddr; }
  uint32_t getPhyAddr() const { return  (uint32_t) phyaddr; }
  uint32_t getHandle()  const { return  (uint32_t) handle; }

	uint32_t alloc_array(uint32_t size_in_bytes);
	uint32_t size_bytes() const { return m_mem_size; }
	uint32_t size() const { return m_mem_size/sizeof(uint32_t); }  // Returns size in words

	// Debug metthods
	void fill(uint32_t value);
	void find_value(uint32_t in_val);
	void detect_used_blocks();

private:
  uint8_t *usraddr    = nullptr;
	uint32_t m_offset   = 0;
  uint32_t m_mem_size = 0;  // Memory size in bytes

  uint32_t handle  = 0;
	uint32_t phyaddr = 0;

  void alloc_mem(uint32_t size_in_bytes);
	void dealloc_mem();


  // Subscript
  uint32_t & operator[] (int i) {
		assert(i >= 0);
		assert(m_mem_size > 0);
		assert(sizeof(uint32_t) * i < m_mem_size);

    uint32_t *base = (uint32_t *) usraddr;
    return (uint32_t&) base[i];
  }
};


BufferObject &getMainHeap();

}  // namespace v3d
}  // namespace QPULib

#endif  // _QPULIB_V3D_BUFFEROBJECT_H_
