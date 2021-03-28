#ifndef _V3DLIB_VC4_DISASM_H
#define _V3DLIB_VC4_DISASM_H

#define DIAG_STRINGIFY(x) #x
#define WIGNORE(flag) _Pragma(DIAG_STRINGIFY(GCC diagnostic ignored #flag))

// NOTE: Following for gcc only
// GCC - should be ok for gcc v6.3.0 and later (currently using v8.3.0)i
//     - explicit v4.x def's dropped

// Disable specific warnings for the encompassed includes
#define DISABLE_COMPILE_TIME_WARNINGS \
  _Pragma("GCC diagnostic push") \
  WIGNORE(-Wconversion) \
  WIGNORE(-Wnarrowing) \
  WIGNORE(-Wsign-conversion) \

// Enable original warnings again
#define ENABLE_COMPILE_TIME_WARNINGS \
  _Pragma("GCC diagnostic pop")

DISABLE_COMPILE_TIME_WARNINGS

#define HAVE_ENDIAN_H
#include "gallium/drivers/vc4/vc4_qpu.h"  // vc4_qpu_disasm()

ENABLE_COMPILE_TIME_WARNINGS


#endif  // _V3DLIB_VC4_DISASM_H
