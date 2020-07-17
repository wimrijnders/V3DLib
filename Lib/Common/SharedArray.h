#ifndef _QPULIB_SHAREDARRAY_H_
#define _QPULIB_SHAREDARRAY_H_

#if !defined(QPU_MODE) && !defined(EMULATION_MODE)
//
// Detect mode, set default if none defined.
// This is the best place to test it in the code, since it's
// the first of the header files to be compiled.
//
#pragma message "WARNING: QPU_MODE and EMULATION_MODE not defined, defaulting to EMULATION_MODE"
#define EMULATION_MODE
#endif

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

  SharedArray(uint32_t n) :
		m_main_array(m_gpu_array_leading?0:n),
		m_gpu_array(m_gpu_array_leading?n:0)
	{}

  uint32_t getAddress() {
		if (m_gpu_array_leading) {
			return m_gpu_array.getAddress();
		} else {
			return m_main_array.getAddress();
		}
  }

  void alloc(uint32_t n) {
		if (m_gpu_array_leading) {
			return m_gpu_array.alloc(n);
		} else {
			return m_main_array.alloc(n);
		}
  }

  T& operator[] (int i) {
		if (m_gpu_array_leading) {
			return m_gpu_array[i];
		} else {
			return m_main_array[i];
		}
	}

	operator VideoCore::SharedArray<T> &() {
		if (m_gpu_array_leading) {
			return m_gpu_array;
		}

		// Transfer data to other buffer
		m_gpu_array.alloc(m_main_array.size);

		for (int n = 0; n < m_main_array.size; ++n) {
			m_gpu_array[n] = m_main_array[n];
		}

		m_gpu_array_leading = true;
		m_main_array.dealloc();
		return m_gpu_array;
	}


	operator Target::SharedArray<T> &() {
		if (!m_gpu_array_leading) {
			return m_main_array;
		}

		// Transfer data to other buffer
		m_main_array.alloc(m_gpu_array.size);

		for (int n = 0; n < m_gpu_array.size; ++n) {
			m_main_array[n] = m_gpu_array[n];
		}

		m_gpu_array_leading = false;
		m_gpu_array.dealloc();
		return m_main_array;
	}

private:
  // Disallow assignment & copying
  void operator=(SharedArray<T> a);
  void operator=(SharedArray<T>& a);
  SharedArray(const SharedArray<T>& a);

	bool m_gpu_array_leading = false;  // If false, use array in  main memory
	Target::SharedArray<T>   m_main_array;
	VideoCore::SharedArray<T> m_gpu_array;
};


}  // QPULib

#endif  // QPU_MODE

#endif  // _QPULIB_SHAREDARRAY_H_
