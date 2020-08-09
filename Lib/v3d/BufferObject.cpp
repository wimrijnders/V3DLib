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
		debug("v3d_unmap() called");

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


/*
void BufferObject::copyFrom(SharedArrayBase const &src, uint32_t offset) {
	assert(m_mem_size >= offset + src.m_mem_size);
  memcpy(usraddr + offset, src.usraddr, src.m_mem_size);
}
*/


}  // namespace v3d
}  // namespace QPULib
