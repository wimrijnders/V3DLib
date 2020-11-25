#ifndef _V3DLIB_HEAP_H_
#define _V3DLIB_HEAP_H_

#if 0  // TODO get rid of this file

//#include <stdio.h>
#include <stdint.h>
#include "Support/basics.h"  // fatal()

namespace V3DLib {

class Heap {
public:
	enum {
		DEFAULT_SIZE = 2*131072
	};

	Heap(const char *name = "", unsigned int heapCapacityInBytes = DEFAULT_SIZE) {
		heapName = name;
		capacity = heapCapacityInBytes;
		size     = 0;
		base     = new uint8_t[capacity];
	}

	~Heap() { delete [] base; }

	void clear() { size = 0; }

	/**
	 * Allocate 'n' elements of type T on the heap
	 */
	template <class T> T* alloc(unsigned long n = 1) {
		unsigned long nbytes = sizeof(T) * n;
		return (T *) alloc_intern(nbytes);
	}

private:
	const char   *heapName = "unnamed";
	unsigned long capacity = 0;
	unsigned long size     = 0;
	uint8_t      *base     = nullptr;

	uint8_t *alloc_intern(unsigned long nbytes = 1) {
		if (size + nbytes >= capacity) {
			char buf[64];
			sprintf(buf, "V3DLib error: heap '%s' is full.\n", heapName);
			fatal(buf);
			return nullptr;
		} 

		uint8_t *p = base + size;
		size += nbytes;
		return p;
	}
};

}  // namespace V3DLib

#endif  // 0

#endif  // _V3DLIB_HEAP_H_
