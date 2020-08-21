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
