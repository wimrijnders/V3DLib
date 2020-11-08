#ifndef _QPULIB_V3D_BUFFEROBJECT_H_
#define _QPULIB_V3D_BUFFEROBJECT_H_
#include <cassert>
#include <vector>
#include <stdint.h>
#include "../Support/debug.h"
#include "Common/BufferObject.h"


namespace QPULib {
namespace v3d {


/**
 * This behaves like an array of uint32_t.
 */
class BufferObject : public QPULib::BufferObject {
public:
	BufferObject(uint32_t size) { alloc_mem(size); }
	BufferObject() {} 
	~BufferObject(); 

  uint32_t getHandle()  const override { return  (uint32_t) handle; }

  void alloc_mem(uint32_t size_in_bytes);

	// Debug methods
	void fill(uint32_t value);
	void find_value(uint32_t in_val);
	void detect_used_blocks();

private:
  uint32_t handle  = 0;

	void dealloc_mem();
	uint32_t &operator[] (int i);
	uint32_t size_word() const { return size()/sizeof(uint32_t); }  // Returns size in words
};


BufferObject &getMainHeap();

}  // namespace v3d
}  // namespace QPULib

#endif  // _QPULIB_V3D_BUFFEROBJECT_H_
