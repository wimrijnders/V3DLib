#include "BufferObject.h"
#include <cassert>
#include <cstdio>
#include <stdlib.h>  // abort()

// When in EMULATION_MODE allocate memory from a pre-allocated pool.
#define EMULATOR_HEAP_SIZE 1024*65536

namespace QPULib {
namespace emu {

/**
 * Allocate heap if not already done so
 */
void BufferObject::alloc_heap() {
	if (emuHeap  == nullptr) {
		emuHeapEnd = 0;
		emuHeap = new uint32_t [EMULATOR_HEAP_SIZE];
	}
}


void BufferObject::check_available(uint32_t n) {
	alloc_heap();

	if (emuHeapEnd + n >= EMULATOR_HEAP_SIZE) {
		printf("QPULib: heap overflow (increase EMULATOR_HEAP_SIZE)\n");
		abort();
	}
}


uint32_t BufferObject::alloc(uint32_t n) {
	check_available(n);

	uint32_t address = emuHeapEnd;
	emuHeapEnd += n;
	return address;
}


uint32_t &BufferObject::operator[] (int i) {
	assert(i >= 0);
	assert(emuHeap != nullptr);

	if (i >= EMULATOR_HEAP_SIZE) {
		printf("QPULib: accessing off end of heap\n");
		exit(EXIT_FAILURE);
	}

   return emuHeap[i];
 }


// Heap used in emulation mode.
BufferObject emuHeap;


}  // namespace emu
}  // namespace QPULib
