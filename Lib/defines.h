///////////////////////////////////////////////////////////////////////////////
// Global defines
///////////////////////////////////////////////////////////////////////////////
#ifndef V3DLIB_DEFINES_H
#define V3DLIB_DEFINES_H

// Detect emulation mode, set it a default if none defined.
// NOTE: QPU_MODE and EMULATION_MODE exclude each other
#if !defined(QPU_MODE) && !defined(EMULATION_MODE)
#pragma message "WARNING: QPU_MODE and EMULATION_MODE not defined, defaulting to EMULATION_MODE"
#define EMULATION_MODE
#endif

#if __GNUC__
#else
#pragma message("WARNING: Using compiler other than GCC. This is not supported (you're on your own)")
#endif


// GCC directives for ARM compilation

#ifdef __aarch64__
  #define ARM64
#else
  #ifdef __arm__    // apparently not set for ARM 64 bits
    #define ARM32
  #endif
#endif

//#if !defined(ARM64) && !defined(ARM32)
//  #pragma message("Compiling on non-ARM platform")
//#else
//  #pragma message("Compiling for ARM")
//#endif

#endif  // V3DLIB_DEFINES_H
