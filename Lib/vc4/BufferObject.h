#ifndef _V3DLIB_VC4_BUFFEROBJECT_H_
#define _V3DLIB_VC4_BUFFEROBJECT_H_
#include "Common/BufferObject.h"

namespace V3DLib {
namespace vc4 {

class BufferObject : public V3DLib::BufferObject {
public:
  ~BufferObject() { dealloc(); }

  void alloc_mem(uint32_t size_in_bytes);

private:
  uint32_t handle = 0;

  void dealloc();
};

BufferObject &getHeap();

}  // namespace vc4
}  // namespace V3DLib

#endif  // _V3DLIB_VC4_BUFFEROBJECT_H_
