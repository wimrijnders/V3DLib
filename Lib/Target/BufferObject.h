#ifndef _QPULIB_TARGET_EMU_BUFFEROBJECT_H_
#define _QPULIB_TARGET_EMU_BUFFEROBJECT_H_
#include "Common/BufferObject.h"

namespace QPULib {
namespace emu {

class BufferObject : public QPULib::BufferObject {
public:
	BufferObject(uint32_t size) { alloc_heap(size); }
	~BufferObject() { dealloc(); }

	uint32_t alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address) override;

	const BufferType buftype = HeapBuffer;

private:
	void alloc_heap(uint32_t size);
	void dealloc() { delete [] arm_base; arm_base = nullptr; }
	void check_available(uint32_t n);
};

BufferObject &getHeap();

}  // namespace emu
}  // namespace QPULib

#endif  // _QPULIB_TARGET_EMU_BUFFEROBJECT_H_
