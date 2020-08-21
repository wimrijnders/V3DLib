#ifndef _QPULIB_VC4_BUFFEROBJECT_H_
#define _QPULIB_VC4_BUFFEROBJECT_H_
#include "Common/BufferObject.h"

namespace QPULib {
namespace vc4 {

class BufferObject : public QPULib::BufferObject {
public:
  ~BufferObject() { dealloc(); }

	uint32_t alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address) override;

private:
  uint32_t handle = 0;

	void dealloc();
};

BufferObject &getHeap();

}  // namespace vc4
}  // namespace QPULib

#endif  // _QPULIB_VC4_BUFFEROBJECT_H_
