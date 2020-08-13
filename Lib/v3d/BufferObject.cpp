#include "BufferObject.h"
//#include <sys/mman.h>
//#include "../debug.h"
#include "v3d.h"

namespace QPULib {
namespace v3d {

BufferObject::~BufferObject() {
	dealloc_mem();
}


/**
 * @param n number of 4-byte elements to allocate (so NOT memory size!)
 */
void BufferObject::alloc_mem(uint32_t n) {
	if (!v3d_open()) {
		assert(false);   // Open device if not already done so
	}

	assert(n > 0);
	assert(handle == 0);

	void *tmp_addr = nullptr;

	if (!v3d_alloc(n, handle, phyaddr, &tmp_addr)) {
		assert(false);
	}
	usraddr = (uint8_t *) tmp_addr;

	assert(handle != 0);
	assert(phyaddr != 0);
	assert(usraddr != nullptr);

	m_mem_size = n;
}


void BufferObject::dealloc_mem() {
	if (usraddr != nullptr) {
		assert(m_mem_size > 0);
		assert(handle > 0);

		// Shouldn't allox be handled in reverse order here???
		// TODO: check

		if (!v3d_unmap(m_mem_size, handle, usraddr)) {
			assert(false);
		}
		//debug("v3d_unmap() called");

		// TODO: what to do about phyaddr here???	
		usraddr = nullptr;
		handle = 0;
		m_mem_size = 0;
	} else {
		// TODO: should  phyaddr be here???	
		assert(m_mem_size == 0);
		assert(handle == 0);
	}
} 


/**
 * @return physical address for array  if allocated, 
 *         0 if could not allocate.
 */
uint32_t BufferObject::alloc_array(uint32_t size_in_bytes) {
	assert(m_mem_size > 0);
	assert(m_offset + size_in_bytes < m_mem_size);
	assert(size_in_bytes % 4 == 0);
	uint32_t prev_offset = m_offset;
	m_offset += size_in_bytes;

	return phyaddr + prev_offset;
}


void BufferObject::fill(uint32_t value) {
	for (uint32_t offset = 0; offset < size(); ++offset) {
		(*this)[offset] = value;
	}
}


/**
 * Scan heap for known value
 *
 * This is a debug method, for testing
 */
void BufferObject::find_value(uint32_t in_val) {
	for (uint32_t offset = 0; offset < size(); ++offset) {
		uint32_t val = (*this)[offset];

		if (val == in_val) {
			printf("Found %u at %u, value: %d - %x\n", in_val, offset, val, val);
		}
	}
}


/**
 * Find assigned blocks within the heap
 *
 * This is a debug method, for testing
 */
void BufferObject::detect_used_blocks() {
	bool have_block = false;
	for (uint32_t offset = 0; offset < size(); ++offset) {
		uint32_t val = (*this)[offset];

		if (have_block) {
			if (val == 0xdeadbeef) {
				printf("block end at %u\n", 4*offset);
				have_block = false;
			}
		} else {
			if (val != 0xdeadbeef) {
				printf("Block at %u, value: %d - %x\n", 4*offset, val, val);
				have_block = true;
			}
		}
	}
}

namespace {

const int HEAP_SIZE = 1024*1024;

BufferObject mainHeap(HEAP_SIZE);

}

BufferObject &getMainHeap() { return mainHeap; }

}  // namespace v3d
}  // namespace QPULib
