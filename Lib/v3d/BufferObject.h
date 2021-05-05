#ifndef _V3DLIB_V3D_BUFFEROBJECT_H_
#define _V3DLIB_V3D_BUFFEROBJECT_H_

#ifdef QPU_MODE

#include <vector>
#include "Common/BufferObject.h"


namespace V3DLib {
namespace v3d {

/**
 * This behaves like an array of uint32_t.
 */
class BufferObject : public V3DLib::BufferObject {
public:
  BufferObject() {} 
  ~BufferObject(); 

  uint32_t getHandle() const override { return  (uint32_t) handle; }

  // Debug methods
  void fill(uint32_t value);
  void find_value(uint32_t in_val);
  void detect_used_blocks();

  static BufferObject &getHeap();

private:
  uint32_t handle  = 0;

  void alloc_mem(uint32_t size_in_bytes) override;
  void dealloc_mem();
  uint32_t &operator[] (int i);
  uint32_t size_word() const { return (uint32_t) (size()/sizeof(uint32_t)); }  // Returns size in words
};

}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE

#endif  // _V3DLIB_V3D_BUFFEROBJECT_H_
