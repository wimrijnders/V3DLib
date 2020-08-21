#ifndef _QPULIB_VC4_BUFFEROBJECT_H_
#define _QPULIB_VC4_BUFFEROBJECT_H_
#include "Common/BufferObject.h"

namespace QPULib {
namespace vc4 {

class BufferObject : public QPULib::BufferObject {
public:
  ~BufferObject() { dealloc(); }

  void alloc_mem(uint32_t size_in_bytes);

private:
  uint32_t handle = 0;

	void dealloc();
};

BufferObject &getHeap();

}  // namespace vc4
}  // namespace QPULib

#endif  // _QPULIB_VC4_BUFFEROBJECT_H_
