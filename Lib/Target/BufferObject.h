#ifndef _QPULIB_TARGET_EMU_BUFFEROBJECT_H_
#define _QPULIB_TARGET_EMU_BUFFEROBJECT_H_
#include "Common/BufferType.h"
#include <stdint.h>

namespace QPULib {
namespace emu {

class BufferObject {
public:
	~BufferObject() { delete [] emuHeap;}

	uint32_t alloc(uint32_t n);


  // Subscript
  uint32_t &operator[] (int i);

	const BufferType buftype = HeapBuffer;

private:
	uint32_t  emuHeapEnd = 0;
	uint32_t *emuHeap    = nullptr;

	void alloc_heap();
	void check_available(uint32_t n);
};

// Heap used in emulation mode.
extern BufferObject emuHeap;

}  // namespace emu
}  // namespace QPULib

#endif  // _QPULIB_TARGET_EMU_BUFFEROBJECT_H_
