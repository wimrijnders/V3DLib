#ifndef _LIB_SUPPORT_DEBUG_H
#define _LIB_SUPPORT_DEBUG_H
#include <signal.h>  // raise(SIGTRAP)

#if defined __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif

#ifdef DEBUG
#include <stdio.h>

#define breakpoint raise(SIGTRAP);

void debug(const char *str);
void warning(const char *str);
void debug_break(const char *str);

#else

#define breakpoint

inline void debug(const char *str) {}
inline void warning(const char *str) {}
inline void debug_break(const char *str) {}

#endif  // DEBUG

void disable_logging();
void assertq(bool cond, const char *msg, bool do_break = false);

#endif  // _LIB_SUPPORT_DEBUG_H
