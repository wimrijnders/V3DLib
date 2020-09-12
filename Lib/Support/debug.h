#ifndef _LIB_DEBUG_H
#define _LIB_DEBUG_H
#include <signal.h>  // raise(SIGTRAP)
#include <cstdio>
#include "Exception.h"


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


/**
 * Alternative for `assert` that throws passed string.
 *
 * Note that this is always enabled, ie. also when not building for DEBUG.
 * See header comment of `fatal()` in `basics.h`
 */
inline void assertq(bool cond, const char *msg) {
	if (!cond) {
		std::string str = "Assertion failed: ";
		str += msg;
		//breakpoint
		throw QPULib::Exception(str);
	}
}

#endif  // _LIB_DEBUG_H
