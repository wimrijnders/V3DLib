#ifndef _V3DLIB_TARGET_EMU_BUFFEROBJECT_H_
#define _V3DLIB_TARGET_EMU_BUFFEROBJECT_H_
#include "Common/BufferObject.h"

namespace V3DLib {
namespace emu {

class BufferObject : public V3DLib::BufferObject {
	using Parent = V3DLib::BufferObject;

public:
	BufferObject(uint32_t size) { alloc_heap(size); }
	~BufferObject() { dealloc(); }

	uint32_t alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address);

	const BufferType buftype = HeapBuffer;

private:
	void alloc_heap(uint32_t size);
	void dealloc() { delete [] arm_base; arm_base = nullptr; }
};

BufferObject &getHeap();

}  // namespace emu
}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_EMU_BUFFEROBJECT_H_
