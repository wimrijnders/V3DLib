#include "SharedArray.h"
#include <sys/mman.h>
#include "../../debug.h"
#include "v3d.h"

namespace QPULib {
namespace v3d {

SharedArrayBase::~SharedArrayBase() {
	breakpoint
	dealloc_mem();
}


/**
 * @param n number of 4-byte elements to allocate (so NOT memory size!)
 */
void SharedArrayBase::alloc_mem(uint32_t n) {
	assert(n > 0);


	if (!v3d_alloc(n, handle, phyaddr, &usraddr)) {
		assert(false);
	}

	m_mem_size = n;
}


void SharedArrayBase::dealloc_mem() {
	if (usraddr != nullptr) {
		assert(m_mem_size > 0);
		assert(handle > 0);

		// Shouldn't allox be handled in reverse order here???
		// TODO: check

		if (!v3d_unmap(m_mem_size, handle, usraddr)) {
			assert(false);
		}

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

}  // namespace v3d
}  // namespace QPULib

