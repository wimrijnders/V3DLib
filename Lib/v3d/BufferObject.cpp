#ifdef QPU_MODE

#include "BufferObject.h"
#include <memory>
#include "Support/basics.h"
#include "Support/Platform.h"  // has_vc4() 
#include "v3d.h"
#include "LibSettings.h"

namespace V3DLib {
namespace v3d {

BufferObject::~BufferObject() {
  dealloc_mem();
}


/**
 *
 */
void BufferObject::alloc_mem(uint32_t size_in_bytes) {
  if (Platform::has_vc4()) fatal("Trying to run v3d code on a vc4");
  if (!v3d_open()) assert(false);   // Only open device if not already done so
  assert(handle == 0);

  void    *tmp_addr = nullptr;
  uint32_t phy_addr = 0;

  if (!v3d_alloc(size_in_bytes, handle, phy_addr, &tmp_addr)) {
    assertq(false, "Failed to allocate v3d shared memory");
  }
  arm_base = (uint8_t *) tmp_addr;

  assert(handle != 0);
  assert(arm_base != nullptr);
  assert(phy_addr != 0);

  set_phy_address(phy_addr);
  set_size(size_in_bytes);
}


void BufferObject::dealloc_mem() {
  if (arm_base != nullptr) {
    assert(size() > 0);
    assert(handle > 0);

    // Shouldn't allocs be handled in reverse order here???
    // TODO: check

    if (!v3d_unmap(size(), handle, arm_base)) {
      assert(false);
    }
    //debug("v3d_unmap() called");

    clear();
    handle = 0;
    arm_base = nullptr;
  } else {
    assert(is_cleared());
    assert(handle == 0);
  }
} 


void BufferObject::fill(uint32_t value) {
  for (uint32_t offset = 0; offset < size_word(); ++offset) {
    (*this)[offset] = value;
  }
}


/**
 * Scan heap for known value
 *
 * This is a debug method, for testing
 */
void BufferObject::find_value(uint32_t in_val) {
  for (uint32_t offset = 0; offset < size_word(); ++offset) {
    uint32_t val = (*this)[offset];

    if (val == in_val) {
      printf("Found %u at %u, value: %d - %x\n", in_val, offset, val, val);
    }
  }
}


/**
 * Find assigned blocks within the heap
 *
 * This is a debug method, for testing
 */
void BufferObject::detect_used_blocks() {
  bool have_block = false;
  for (uint32_t offset = 0; offset < size_word(); ++offset) {
    uint32_t val = (*this)[offset];

    if (have_block) {
      if (val == 0xdeadbeef) {
        printf("block end at %u\n", 4*offset);
        have_block = false;
      }
    } else {
      if (val != 0xdeadbeef) {
        printf("Block at %u, value: %d - %x\n", 4*offset, val, val);
        have_block = true;
      }
    }
  }
}

namespace {

// Defined as a smart ptr to avoid issues on program init
std::unique_ptr<BufferObject> mainHeap;

}


BufferObject &BufferObject::getHeap() {
  if (!Platform::has_vc4()) {
    if (!mainHeap) {
      //debug("Allocating main heap v3d\n");
      mainHeap.reset(new BufferObject());
      mainHeap->alloc(LibSettings::heap_size());
    }
  }

  return *mainHeap;
}


// Subscript
uint32_t &BufferObject::operator[] (int i) {
  assert(i >= 0);
  assert(size() > 0);
  assertq(sizeof(uint32_t) * i < size(), "Index out of range", true);

  uint32_t *base = (uint32_t *) arm_base;
  return (uint32_t&) base[i];
}

}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE
