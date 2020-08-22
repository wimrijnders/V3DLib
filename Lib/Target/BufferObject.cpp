#include "BufferObject.h"
#include <cassert>
#include <memory>
#include <cstdio>
#include <stdlib.h>  // abort()
#include "../Support/debug.h"

namespace QPULib {
namespace emu {

namespace {

// Defined as a smart ptr to avoid issues on program init
std::unique_ptr<BufferObject> emuHeap;

}


/**
 * Allocate heap if not already done so
 */
void BufferObject::alloc_heap(uint32_t size) {
	assert(arm_base  == nullptr);

	emuHeapEnd = 0;
	arm_base = new uint8_t [size];
	m_size = size;
}



void BufferObject::check_available(uint32_t n) {
	assert(arm_base != nullptr);

	if (emuHeapEnd + n >= m_size) {
		printf("QPULib: heap overflow (increase heap size)\n");
		abort();
	}
}


uint32_t BufferObject::alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address) {
	assert(arm_base != nullptr);
	check_available(size_in_bytes);

	uint32_t address = emuHeapEnd;
	emuHeapEnd += size_in_bytes;
	array_start_address = arm_base;
	return address;
}


BufferObject &getHeap() {
	if (!emuHeap) {
		debug("Allocating emu heap v3d\n");
		emuHeap.reset(new BufferObject(BufferObject::DEFAULT_HEAP_SIZE));
	}

	return *emuHeap;
}


}  // namespace emu
}  // namespace QPULib
