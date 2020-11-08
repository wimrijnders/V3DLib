#ifndef _QPULIB_COMMON_BUFFEROBJECT_H_
#define _QPULIB_COMMON_BUFFEROBJECT_H_
#include <stdint.h>
//#include <vector>
#include "Common/BufferType.h"
#include "Support/HeapManager.h"

// NOTE: QPU_MODE and EMULATION_MODE exclude each other
#if !defined(QPU_MODE) && !defined(EMULATION_MODE)
//
// Detect mode, set default if none defined.
// This is the best place to test it in the code, since it's
// the first of the header files to be compiled.
//
#pragma message "WARNING: QPU_MODE and EMULATION_MODE not defined, defaulting to EMULATION_MODE"
#define EMULATION_MODE
#endif


namespace QPULib {

class BufferObject : public HeapManager {
public:
	BufferObject() {}
	BufferObject(BufferObject *buffer) = delete;
	virtual ~BufferObject() {}

	virtual uint32_t alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address);
  virtual uint32_t getHandle() const;

	static const int DEFAULT_HEAP_SIZE = 5*1024*1024;

	uint8_t *usr_address() { return arm_base; }

protected:
	uint8_t *arm_base = nullptr;

private:
	// Disallow assignment
	void operator=(BufferObject a);
	void operator=(BufferObject& a);

};

BufferObject &getBufferObject();

}  // namespace QPULib

#endif  // _QPULIB_COMMON_BUFFEROBJECT_H_
