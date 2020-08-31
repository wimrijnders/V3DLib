#ifndef _QPULIB_COMMON_BUFFEROBJECT_H_
#define _QPULIB_COMMON_BUFFEROBJECT_H_
#include <stdint.h>
#include <vector>
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
	BufferObject();
	BufferObject(BufferObject *buffer) = delete;
	virtual ~BufferObject() {}

	virtual uint32_t alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address);
  virtual uint32_t getHandle() const;

	static const int DEFAULT_HEAP_SIZE = 5*1024*1024;

	uint32_t size();
	bool empty() const { return m_offset == 0; }
	uint8_t *usr_address() { return arm_base; }
	uint32_t phy_address() { return phyaddr; }

	void dealloc_array(uint32_t in_phyaddr, uint32_t size);

#ifdef DEBUG
	int num_free_ranges() const { return m_free_ranges.size(); }
#endif

protected:
  uint32_t m_size   = 0;  // Total allocated size of BufferObject
	uint8_t *arm_base = nullptr;
	uint32_t phyaddr  = 0;
	uint32_t m_offset = 0;

private:
	// Disallow assignment
	void operator=(BufferObject a);
	void operator=(BufferObject& a);

	struct FreeRange {
		FreeRange(uint32_t l, uint32_t r) : left(l), right(r) {}  // required by std::vector

		uint32_t left  = 0;
		uint32_t right = 0;
	};

	std::vector<FreeRange> m_free_ranges;
};

BufferObject &getBufferObject();

}  // namespace QPULib

#endif  // _QPULIB_COMMON_BUFFEROBJECT_H_
