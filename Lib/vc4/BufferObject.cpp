#include "BufferObject.h"
#include <cassert>
#include <stdio.h>
#include "Mailbox.h"
#include "vc4.h"
#include "../Support/Platform.h"  // has_vc4() 
#include "../Support/basics.h"
#include "../Support/debug.h"

#define GPU_MEM_FLG 0xC // cached=0xC; direct=0x4
#define GPU_MEM_MAP 0x0 // cached=0x0; direct=0x20000000

namespace V3DLib {
namespace vc4 {

namespace {

BufferObject heap;

}

/**
 * Allocate GPU memory and map it into ARM address space
 */
void BufferObject::alloc_mem(uint32_t size_in_bytes) {
	int mb = getMailbox();  // Mailbox, for talking to vc4

	// Allocate memory
	handle = mem_alloc(mb, size_in_bytes*4, 4096, GPU_MEM_FLG);
	if (!handle) {
		fatal("Failed to allocate GPU memory.");
	}

	uint32_t phyaddr = mem_lock(mb, handle);
	arm_base =  (uint8_t *) mapmem(BUS_TO_PHYS(phyaddr + GPU_MEM_MAP), size_in_bytes);

	set_size(size_in_bytes);
	set_phy_address(phyaddr);
}


// Deallocation
void BufferObject::dealloc() {
	if (arm_base == nullptr) {
		assert(handle == 0);
		assert(empty());
		return;
	}

	//debug("Deallocating memory for vc4 bo");

	int mb = getMailbox();  // Mailbox, for talking to vc4

	// Free memory
	if (arm_base) unmapmem(arm_base, size());
	if (handle) {
		mem_unlock(mb, handle);
		mem_free(mb, handle);
	}

	handle = 0;
	clear();
	arm_base = nullptr;
}


BufferObject &getHeap() {
	if (Platform::instance().has_vc4) {
		if (heap.size() == 0) {
			//debug("Allocating main heap vc4\n");
			heap.alloc_mem(BufferObject::DEFAULT_HEAP_SIZE);
		}
	}

	return heap;
}

}  // namespace vc4
}  // namespace V3DLib
