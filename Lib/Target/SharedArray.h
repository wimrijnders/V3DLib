////////////////////////////////////////////////////////////////////////////////
// Emulator version of SharedArray
////////////////////////////////////////////////////////////////////////////////

#ifndef _QPULIB_EMULATOR_SHAREDARRAY_H_
#define _QPULIB_EMULATOR_SHAREDARRAY_H_

extern uint32_t emuHeapEnd;
extern int32_t* emuHeap;


namespace QPULib {
namespace Target {

// ============================================================================
// Emulation mode
// ============================================================================

// When in EMULATION_MODE allocate memory from a pre-allocated pool.
#define EMULATOR_HEAP_SIZE 1024*65536


// Implementation
template <typename T> class SharedArray {
 private:
   // Disallow assignment
   void operator=(SharedArray<T> a);
   void operator=(SharedArray<T>& a);

 public:

  uint32_t address;
  uint32_t size;

  // Allocation
  void alloc(uint32_t n) {
    if (emuHeap == NULL) {
      emuHeapEnd = 0;
      emuHeap = new int32_t [EMULATOR_HEAP_SIZE];
    }
    if (emuHeapEnd+n >= EMULATOR_HEAP_SIZE) {
      printf("QPULib: heap overflow (increase EMULATOR_HEAP_SIZE)\n");
      abort();
    }
    else {
      address = emuHeapEnd;
      emuHeapEnd += n;
      size = n;
    }
  }

  // Constructor
  SharedArray(uint32_t n) {
    alloc(n);
  }

  uint32_t getAddress() {
    return address*4;
  }

  T* getPointer() {
    return (T*) &emuHeap[address];
  }

  // Deallocation (does nothing in emulation mode)
  void dealloc() {}

  // Subscript
  T& operator[] (int i) {
    if (address+i >= EMULATOR_HEAP_SIZE) {
      printf("QPULib: accessing off end of heap\n");
      exit(EXIT_FAILURE);
    }
    else
      return (T&) emuHeap[address+i];
  }
};


}  // namespace Target
}  // namespace QPULib



#endif  // _QPULIB_EMULATOR_SHAREDARRAY_H_
