#ifndef _V3DLIB_COMMON_BUFFEROBJECT_H_
#define _V3DLIB_COMMON_BUFFEROBJECT_H_
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


namespace V3DLib {

class BufferObject : public HeapManager {
public:
	BufferObject() {}
	BufferObject(BufferObject *buffer) = delete;
	virtual ~BufferObject() {}

  virtual uint32_t getHandle() const;

	uint32_t alloc_array(uint32_t size_in_bytes, uint8_t *&array_start_address);
	void dealloc_array(uint32_t in_phyaddr, uint32_t in_size);

	static const int DEFAULT_HEAP_SIZE = 5*1024*1024;

	uint32_t phy_address() const { return phyaddr; }
	uint8_t *usr_address() { return arm_base; }

protected:
	uint8_t *arm_base = nullptr;

	void set_phy_address(uint32_t val);
	void clear();
	bool is_cleared() const;

private:
	// Disallow assignment
	void operator=(BufferObject a);
	void operator=(BufferObject& a);

	uint32_t phyaddr  = 0;
};

BufferObject &getBufferObject();

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_BUFFEROBJECT_H_
