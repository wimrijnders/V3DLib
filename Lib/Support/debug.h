#ifndef _V3DLIB_SUPPORT_DEBUG_H
#define _V3DLIB_SUPPORT_DEBUG_H
#include <signal.h>  // raise(SIGTRAP)
#include <string>

#if defined __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif

#ifdef DEBUG

#define breakpoint raise(SIGTRAP);

void debug(const char *str);
void warning(const char *str);
inline void warning(std::string const &str) { return warning(str.c_str()); }
void debug_break(const char *str);

#else

#define breakpoint

inline void debug(const char *str) {}
inline void warning(const char *str) {}
inline void debug_break(const char *str) {}

#endif  // DEBUG

void error(const char *str, bool do_throw = false);
inline void error(std::string const &msg, bool do_throw = false) { error(msg.c_str(), do_throw); }

void disable_logging();
void assertq(bool cond, const char *msg, bool do_break = false);

inline void assertq(bool cond, std::string const &msg, bool do_break = false) {
  assertq(cond, msg.c_str(), do_break);
}

inline void assertq(std::string const &msg, bool do_break = false) { assertq(false, msg, do_break); }

#endif  // _V3DLIB_SUPPORT_DEBUG_H
