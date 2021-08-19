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


/**
 * Get actual offset within the heap for given index of current shared array.
 */
uint32_t BaseSharedArray::phy(uint32_t i) {
  assert(m_phyaddr % m_element_size == 0);
  int index = (int) (i - ((uint32_t) m_phyaddr/m_element_size));
  assert(index >= 0);
  return (uint32_t) index;
}

}  // namespace V3DLib

