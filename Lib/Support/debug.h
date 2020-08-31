#ifndef _LIB_DEBUG_H
#define _LIB_DEBUG_H
#include <signal.h>  // raise(SIGTRAP)
//#include <stdlib.h>  // abort, NULL
#include <cstdio>


#if defined __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif


#ifdef DEBUG
#include <stdio.h>

#define breakpoint raise(SIGTRAP);

inline void debug(const char *str) {
	printf("DEBUG: %s\n", str);
}


inline void warning(const char *str) {
	printf("WARNING: %s\n", str);
}


inline void debug_break(const char *str) {
	printf("DEBUG: %s\n", str);
	breakpoint
}

#else

#define breakpoint

inline void debug(const char *str) {}
inline void warning(const char *str) {}
inline void debug_break(const char *str) {}

#endif  // DEBUG

#endif  // _LIB_DEBUG_H
