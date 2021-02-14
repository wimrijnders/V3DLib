#include "SharedArray.h"

namespace V3DLib {

BaseSharedArray::BaseSharedArray(BufferObject *heap, uint32_t element_size) :
  m_heap(heap),
  m_element_size(element_size)
{}


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
void BaseSharedArray::alloc(uint32_t n) {
  assert(!allocated());
  assert(n > 0);
  assert(m_element_size > 0);

  if (m_heap == nullptr) {
    m_heap = &getBufferObject();
  }

  m_phyaddr = m_heap->alloc_array((uint32_t) (m_element_size*n), m_usraddr);
  m_size = n;
  assert(allocated());
}


/**
 * Forget the allocation and size and notify the underlying heap.
 */
void BaseSharedArray::dealloc() {
  assert(m_element_size > 0);

  if (m_size > 0) {
    assert(allocated());
    assert(m_heap != nullptr);
    if (!m_is_heap_view) { 
      m_heap->dealloc_array(m_phyaddr, (uint32_t) (m_element_size*m_size));
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
 *
 * TODO my god this is so ugly, can we please clean it up??
 */
uint32_t *BaseSharedArray::getPointer() {
#ifdef QPU_MODE
  if (Platform::has_vc4()) {
#ifdef ARM64
    // This part is useless, it will in fact never be called.
    // It's only here for successful compilation.
    return (uint32_t *) (uint64_t) m_phyaddr;  // Double cast to prevent conversion warning
#else
    return (uint32_t *) m_phyaddr;
#endif
  } else {
    return (uint32_t *) m_usraddr;
  }
#else
  return (uint32_t *) m_usraddr;
#endif
}


void BaseSharedArray::heap_view(BufferObject &heap) {
  assert(!allocated());
  assert(m_heap == nullptr);

  m_heap = &heap;
  m_is_heap_view = true;
  m_size = m_heap->size();
  assert(m_size > 0);
  m_usraddr = m_heap->usr_address();
  m_phyaddr = m_heap->phy_address();
}

}  // namespace V3DLib

