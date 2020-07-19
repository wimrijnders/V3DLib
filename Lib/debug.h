#ifndef _LIB_DEBUG_H
#define _LIB_DEBUG_H
#include <cassert>
#include <signal.h>  // raise(SIGTRAP);

#ifdef DEBUG

#define breakpoint raise(SIGTRAP);

#else

#define breakpoint

#endif  // DEBUG

#endif  // _LIB_DEBUG_H
