#ifndef _QPULIB_VIDEOCORE_SHAREDARRAY_H_
#define _QPULIB_VIDEOCORE_SHAREDARRAY_H_
#include <cassert>
#include <cstdlib>  // exit(), EXIT_FAILURE
#include <stdint.h>
#include <stdio.h>
#include "Mailbox.h"
#include "vc4.h"

#ifdef QPU_MODE

namespace QPULib {
namespace vc4 {

// ============================================================================
// Not emulation mode
// ============================================================================

#define GPU_MEM_FLG 0xC // cached=0xC; direct=0x4
#define GPU_MEM_MAP 0x0 // cached=0x0; direct=0x20000000

template <typename T> class SharedArray {
 private:
  // Disallow assignment & copying
  void operator=(SharedArray<T> a);
  void operator=(SharedArray<T>& a);
  SharedArray(const SharedArray<T>& a);

  uint32_t handle = 0;
  uint32_t m_size = 0;
  void* arm_base = NULL;
  void* gpu_base = NULL;

 public:

  uint32_t size() { return m_size; }

  /**
	 * Allocate GPU memory and map it into ARM address space
	 */
  void alloc(uint32_t n) {
		if (n == 0) {
			assert(m_size == 0 && handle == 0 && arm_base == NULL && gpu_base == NULL);
			return;
		}

    int mb = getMailbox();  // Mailbox, for talking to vc4

    // Allocate memory
    handle = mem_alloc(mb, n*4, 4096, GPU_MEM_FLG);
    if (!handle) {
      fprintf(stderr, "Failed to allocate GPU memory.");
      exit(EXIT_FAILURE);
    }
    m_size = n;
    gpu_base = (void*) mem_lock(mb, handle);
    arm_base = mapmem(BUS_TO_PHYS((uint32_t) gpu_base+GPU_MEM_MAP), n*4);
  }

  // Constructor
  SharedArray() {}

  // Constructor
  SharedArray(uint32_t n) : m_size(n) {
		assert(n >= 0);
	  alloc(n);
  }  

  uint32_t getAddress() {
    return (uint32_t) gpu_base;
  }

  T* getPointer() {
    return (T*) gpu_base;
  }

  // Deallocation
  void dealloc() {
    int mb = getMailbox();  // Mailbox, for talking to vc4

    // Free memory
    if (arm_base) unmapmem(arm_base, m_size);
    if (handle) {
      mem_unlock(mb, handle);
      mem_free(mb, handle);
    }
    m_size = handle = 0;
    gpu_base = NULL;
    arm_base = NULL;
  }

  // Subscript
  inline T& operator[] (int i) {
		assert(i >= 0);
		assert(m_size > 0);
		assert(i < m_size);

    uint32_t* base = (uint32_t*) arm_base;
    return (T&) base[i];
  }

  // Destructor
  ~SharedArray() {
    if (arm_base != NULL) dealloc();
  }
};

}  // namespace vc4
}  // namespace QPULib

#endif  // QPU_MODE

#endif // _QPULIB_VIDEOCORE_SHAREDARRAY_H_
