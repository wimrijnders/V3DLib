#include "BufferObject.h"
#include "../Support/debug.h"
#include "../Support/Platform.h"  // has_vc4() 
#include "v3d.h"

namespace QPULib {
namespace v3d {

BufferObject::~BufferObject() {
	dealloc_mem();
}


/**
 * @param n number of 4-byte elements to allocate (so NOT memory size!)
 */
void BufferObject::alloc_mem(uint32_t size_in_bytes) {
	if (Platform::instance().has_vc4) {
		fatal("Trying to run v3d code on a vc4");
	}

	if (!v3d_open()) {
		assert(false);   // Open device if not already done so
	}

	assert(size_in_bytes > 0);
	assert(handle == 0);

	void *tmp_addr = nullptr;

	if (!v3d_alloc(size_in_bytes, handle, phyaddr, &tmp_addr)) {
		assert(false);
	}
	arm_base = (uint8_t *) tmp_addr;

	assert(handle != 0);
	assert(phyaddr != 0);
	assert(arm_base != nullptr);

	m_size = size_in_bytes;
}


void BufferObject::dealloc_mem() {
	if (arm_base != nullptr) {
		assert(m_size > 0);
		assert(handle > 0);

		// Shouldn't allocs be handled in reverse order here???
		// TODO: check

		if (!v3d_unmap(m_size, handle, arm_base)) {
			assert(false);
		}
		//debug("v3d_unmap() called");

		m_size = 0;
		handle = 0;
		arm_base = nullptr;
		phyaddr = 0;
		m_offset = 0;
	} else {
		assert(m_size == 0);
		assert(handle == 0);
		assert(phyaddr == 0);
		assert(m_offset == 0);
	}
} 


/**
 * @return physical address for array  if allocated, 
 *         0 if could not allocate.
 */
uint32_t BufferObject::alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address) {
	assert(m_size > 0);
	assert(m_offset + size_in_bytes <= m_size);
	assert(size_in_bytes % 4 == 0);

	uint32_t prev_offset = m_offset;

	array_start_address = arm_base + m_offset;
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

BufferObject mainHeap;

}

BufferObject &getMainHeap() {
	if (!Platform::instance().has_vc4) {
		if (mainHeap.size() == 0) {
			//debug("Allocating main heap v3d\n");
			mainHeap.alloc_mem(HEAP_SIZE);
		}
	}

	return mainHeap;
}


// Subscript
uint32_t &BufferObject::operator[] (int i) {
	assert(i >= 0);
	assert(m_size > 0);
	assert(sizeof(uint32_t) * i < m_size);

	uint32_t *base = (uint32_t *) arm_base;
	return (uint32_t&) base[i];
}

}  // namespace v3d
}  // namespace QPULib
