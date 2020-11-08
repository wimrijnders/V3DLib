#include "BufferObject.h"
#include "../Support/Platform.h"
#include "BufferType.h"
#include "../Target/BufferObject.h"
#include "../vc4/BufferObject.h"
#include "../v3d/BufferObject.h"

namespace QPULib {

uint32_t BufferObject::alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address) {
	uint32_t new_offset;
	uint32_t ret = HeapManager::alloc_array(size_in_bytes, new_offset);
	array_start_address = arm_base + new_offset;
	return ret;
}


uint32_t BufferObject::getHandle() const {
	assert(false);  // Only use for v3d
	return 0;
}


BufferObject &getBufferObject() {
	if (Platform::instance().use_main_memory()) {
		return emu::getHeap();
	} else if (Platform::instance().has_vc4) {
		return vc4::getHeap();
	} else {
		return v3d::getMainHeap();
	}
}

}  // namespace QPULib
