#ifndef _LIB_DEBUG_H
#define _LIB_DEBUG_H
#include <cassert>
#include <signal.h>  // raise(SIGTRAP);

#ifdef DEBUG
#include <stdio.h>

#define breakpoint raise(SIGTRAP);

inline void debug(const char *str) {
	printf("%s\n", str);
}

#else

#define breakpoint

inline void debug(const char *str) {}

#endif  // DEBUG

#endif  // _LIB_DEBUG_H
