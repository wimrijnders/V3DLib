#include "BufferObject.h"
#include <cassert>
#include <cstdio>
#include <stdlib.h>  // abort()

// When in EMULATION_MODE allocate memory from a pre-allocated pool.
#define EMULATOR_HEAP_SIZE 1024*65536

namespace QPULib {
namespace emu {

namespace {

BufferObject emuHeap;

}


/**
 * Allocate heap if not already done so
 */
void BufferObject::alloc_heap() {
	if (arm_base  == nullptr) {
		emuHeapEnd = 0;
		arm_base = new uint8_t [EMULATOR_HEAP_SIZE];
	}
}



void BufferObject::check_available(uint32_t n) {
	alloc_heap();

	if (emuHeapEnd + n >= EMULATOR_HEAP_SIZE) {
		printf("QPULib: heap overflow (increase EMULATOR_HEAP_SIZE)\n");
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
	return emuHeap;
}


}  // namespace emu
}  // namespace QPULib
