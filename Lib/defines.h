#ifndef V3DLIB_DEFINES_H
#define V3DLIB_DEFINES_H

#if __GNUC__
#else
#pragma message("WARNING: Using compiler other than GCC. This is not supported (you're on your own)")
#endif


// GCC directive for ARM compilation
#ifdef __arm__
  #pragma message("Compiling for ARM")

  #ifdef __aarch64__
    #define ARM64
  #else
    #define ARM32
  #endif
#else
  #pragma message("Compiling on non-ARM platform")
#endif

#endif  // V3DLIB_DEFINES_H
