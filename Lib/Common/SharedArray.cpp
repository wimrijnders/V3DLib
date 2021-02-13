#include "SharedArray.h"

namespace V3DLib {

bool BaseSharedArray::allocated() const {
  if (m_size > 0) {
    assert(m_heap != nullptr);
    // assert(m_phyaddr > 0);  // Can be 0 for emu
    assert(m_usraddr != nullptr);
    return true;
  } else {
    assert(m_phyaddr == 0);
    assert(m_usraddr == nullptr);
    assert(!m_is_heap_view);
    return false;
  }
}


/**
 * @param n number of 4-byte elements to allocate (so NOT memory size!)
 */
void BaseSharedArray::alloc(uint32_t n, uint32_t element_size) {
  assert(!allocated());
  assert(n > 0);

  if (m_heap == nullptr) {
    m_heap = &getBufferObject();
  }

  m_phyaddr = m_heap->alloc_array((uint32_t) (element_size*n), m_usraddr);
  m_size = n;
  assert(allocated());
}


/**
 * Forget the allocation and size and notify the underlying heap.
 */
void BaseSharedArray::dealloc(uint32_t element_size) {
  if (m_size > 0) {
    assert(allocated());
    assert(m_heap != nullptr);
    if (!m_is_heap_view) { 
      m_heap->dealloc_array(m_phyaddr, (uint32_t) (element_size*m_size));
    }

    m_phyaddr = 0;
    m_size = 0;
    m_usraddr = nullptr;
    m_is_heap_view = false;
  } else {
    assert(!allocated());
  }
}


/**
 * Get starting address of the section in question
 *
 * Needed for vc4, emulator and interpreter mode.
 */
void *BaseSharedArray::getPointer() {
#ifdef QPU_MODE
  if (Platform::has_vc4()) {
    return (void *) m_phyaddr;  // TODO fix conversion warning for ARM 64 bits
  } else {
    return (void *) m_usraddr;
  }
#else
  return (void *) m_usraddr;
#endif
}

}  // namespace V3DLib

