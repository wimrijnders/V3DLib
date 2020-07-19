/**

The emulator heap is tightly coupled to the emulator (and interpreter).
Notably, the SharedArray's in the heap are assumed to be in the same contiguous address space.
This is not the case for QPU mode SharedArray's

There is no way to use the QPU mode SharedArray without a serious rewrite.

*/
#ifndef _QPULIB_SHAREDARRAY_H_
#define _QPULIB_SHAREDARRAY_H_
#include <signal.h>  // raise(SIGTRAP);


namespace QPULib {

enum BufferType : int {
	HeapBuffer,
	Vc4Buffer //,
//	V3dBuffer
};

}


#if !defined(QPU_MODE) && !defined(EMULATION_MODE)
//
// Detect mode, set default if none defined.
// This is the best place to test it in the code, since it's
// the first of the header files to be compiled.
//
#pragma message "WARNING: QPU_MODE and EMULATION_MODE not defined, defaulting to EMULATION_MODE"
#define EMULATION_MODE
#endif

#pragma message "Debugging"

#include "../Target/SharedArray.h"

#ifndef QPU_MODE		// NOTE: QPU_MODE and EMULATION_MODE exclude each other

namespace QPULib {
// Use main memory SharedArray directly
template <typename T>
using SharedArray=Target::SharedArray<T>;
}  // QPULib

#else  // QPU_MODE
#include "../VideoCore/SharedArray.h"

//template <typename T>
//using SharedArray=QPULib::VideoCore::SharedArray<T>;
#endif  // QPU_MODE


#ifdef QPU_MODE

/*
Not working yet.

This bricked the Pi3 during testing. Needs more debugging
*/

namespace QPULib {

/**
 * Combined SharedArray definition which both EMU and QPU to be used
 * in a given program.
 *
 * ----------------------------------------------------------------------------
 * NOTE
 * ====
 *
 * * Following scenario's must be taken into account:
 *
 *    - non-pi, EMU -> main mem
 *    - pi, vc4     -> main + gpu mem  (main for emulator)
 *    - pi, vc6     -> main mem
 */
template <typename T>
class SharedArray {
public:
	SharedArray() {}

  SharedArray(uint32_t n, BufferType buftype = HeapBuffer) :
		m_buftype(buftype),
		m_size(n),
		m_main_array((buftype == HeapBuffer)?n:0),
		m_gpu_array((buftype == Vc4Buffer) ?n:0)
	{
		assert(m_size > 0);
	}


  uint32_t size() {
raise(SIGTRAP);
		return m_size;
	}


  uint32_t getAddress() {
		uint32_t ret;

		switch(m_buftype) {
		case HeapBuffer:
			ret = m_main_array.getAddress();
			break;
		case Vc4Buffer:
raise(SIGTRAP);
			ret = m_gpu_array.getAddress();
			break;
		}

		return ret;
  }


	void setType(BufferType buftype) {
		switch(buftype) {
		case HeapBuffer:
			switch(m_buftype) {
			case HeapBuffer:
				// Nothing to do
				break;
			case Vc4Buffer:
				moveTo(m_gpu_array, m_main_array);
				break;
			}
			break;
		case Vc4Buffer:
			switch(m_buftype) {
			case HeapBuffer:
				moveTo(m_main_array, m_gpu_array);
				break;
			case Vc4Buffer:
				// Nothing to do
				break;
			}
			break;
		}

		m_buftype = buftype;
	}


  void alloc(uint32_t n) {
		assert(n > 0);
		assert(m_size == 0);

		switch(m_buftype) {
		case HeapBuffer:
			assert(m_main_array.size() == 0);
			m_main_array.alloc(n);
			break;
		case Vc4Buffer:
raise(SIGTRAP);
			assert(m_gpu_array.size() == 0);
			m_gpu_array.alloc(n);
			break;
		}

		m_size = n;
  }

  T& operator[] (int i) {
		assert(i >= 0 && i < m_size);

		T *ret = nullptr;

		switch(m_buftype) {
		case HeapBuffer:
			ret = &m_main_array[i];
			break;
		case Vc4Buffer:
			ret = &m_gpu_array[i];
			break;
		}

		assert(ret != nullptr);
		return *ret;
	}

	operator VideoCore::SharedArray<T> &() {
		setType(Vc4Buffer);	
		return m_gpu_array;
	}


	operator Target::SharedArray<T> &() {
raise(SIGTRAP);
		setType(HeapBuffer);	
		return m_main_array;
	}

private:
  // Disallow assignment & copying
  void operator=(SharedArray<T> a);
  void operator=(SharedArray<T>& a);
  SharedArray(const SharedArray<T>& a);

	BufferType m_buftype = HeapBuffer;
  uint32_t m_size = 0;

	Target::SharedArray<T>   m_main_array;
	VideoCore::SharedArray<T> m_gpu_array;


	template<typename Src, typename Dst>
	void moveTo(Src &src, Dst &dst) {
		if (src.size() == 0) {
raise(SIGTRAP);
			// Nothing to transfer
			assert(dst.size() == 0);
			return;
		}

		// Transfer data to other buffer
raise(SIGTRAP);
		assert(dst.size() == 0);
		dst.alloc(src.size());

		for (int n = 0; n < src.size(); ++n) {
			dst[n] = src[n];
		}

		src.dealloc();
	}
};


}  // QPULib

#endif  // QPU_MODE

#endif  // _QPULIB_SHAREDARRAY_H_
