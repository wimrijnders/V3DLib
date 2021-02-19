#include "BufferObject.h"
#include "Support/Platform.h"
#include "Support/debug.h"
#include "BufferType.h"
#include "Target/BufferObject.h"
#include "vc4/BufferObject.h"
#include "v3d/BufferObject.h"

namespace V3DLib {

/**
 * @param size_in_bytes        requested size of memory to allocate 
 * @param array_start_address  out parameter; memory address of the newly allocated memory in the heap
 *
 * @return physical address of the newly allocated memory in the heap
 */
uint32_t BufferObject::alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address) {
  int new_offset = HeapManager::alloc_array(size_in_bytes);
  assert(new_offset >= 0);
  array_start_address = arm_base + (uint32_t) new_offset;
  return phy_address() + (uint32_t) new_offset;
}


void BufferObject::dealloc_array(uint32_t in_phyaddr, uint32_t in_size) {
  assert(phy_address() <= in_phyaddr && in_phyaddr < (phy_address() + size()));
  HeapManager::dealloc_array(in_phyaddr - phy_address(), in_size);
}


uint32_t BufferObject::getHandle() const {
  assertq(false, "getHandle(): the base version of this method should never be called, only the v3d override.\n"
                 "Perhaps you are using main memory for the buffer object?", true);
  return 0;
}


void BufferObject::set_phy_address(uint32_t val) {
  assert(val > 0);
  assert(phyaddr == 0);  // Only allow initial size setting for now
  phyaddr = val;
}


void BufferObject::clear() {
  phyaddr = 0;
  HeapManager::clear();
}


bool BufferObject::is_cleared() const {
  if  (size() == 0) {
    assert(phyaddr == 0);
  }

  return HeapManager::is_cleared();
}


BufferObject &getBufferObject() {
  if (Platform::use_main_memory()) {
    return emu::getHeap();
  } else if (Platform::has_vc4()) {
    return vc4::getHeap();
  } else {
    return v3d::getMainHeap();
  }
}

}  // namespace V3DLib
