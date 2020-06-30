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

#ifndef QPU_MODE		// NOTE: QPU_MODE and EMULATION_MODE exclude each other
#include "../Target/SharedArray.h"
#else  // QPU_MODE
#include "../VideoCore/SharedArray.h"
#endif  // QPU_MODE

#endif  // _QPULIB_SHAREDARRAY_H_
