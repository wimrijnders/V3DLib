#include "BufferObject.h"
#include "../Support/Platform.h"
#include "BufferType.h"
#include "../Target/BufferObject.h"
#include "../vc4/BufferObject.h"
#include "../v3d/BufferObject.h"

namespace QPULib {


uint32_t BufferObject::size() {
	return m_size;
}


/**
 * @return physical address for array if allocated, 
 *         0 if could not allocate.
 */
uint32_t BufferObject::alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address) {
	assert(m_size > 0);
	assert(size_in_bytes % 4 == 0);

	if (m_offset + size_in_bytes >= m_size) {
		return 0;
	}

	uint32_t prev_offset = m_offset;

	array_start_address = arm_base + m_offset;
	m_offset += size_in_bytes;
	return phyaddr + prev_offset;
}


uint32_t BufferObject::getHandle() const {
	assert(false);  // Only use for v3d
	return 0;
}


BufferObject &getBufferObject() {
	if (Platform::instance().emulator_only) {
		return emu::getHeap();
	} else if (Platform::instance().has_vc4) {
		return vc4::getHeap();
	} else {
		return v3d::getMainHeap();
	}
}

}  // namespace QPULib
