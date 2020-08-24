#ifndef _QPULIB_COMMON_BUFFEROBJECT_H_
#define _QPULIB_COMMON_BUFFEROBJECT_H_
#include <stdint.h>
#include "Common/BufferType.h"

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

class BufferObject {
public:
	virtual ~BufferObject() {}

	virtual uint32_t alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address);
  virtual uint32_t getHandle() const;

	static const int DEFAULT_HEAP_SIZE = 5*1024*1024;

	uint32_t size();
	uint8_t *usr_address() { return arm_base; }
	uint32_t phy_address() { return phyaddr; }

protected:
  uint32_t m_size   = 0;  // Total allocated size of BufferObject
	uint8_t *arm_base = nullptr;
	uint32_t phyaddr = 0;
	uint32_t m_offset   = 0;
};

BufferObject &getBufferObject();

}  // namespace QPULib

#endif  // _QPULIB_COMMON_BUFFEROBJECT_H_
