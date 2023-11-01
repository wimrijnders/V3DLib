#ifndef _V3DLIB_SUPPORT_BASICS_H
#define _V3DLIB_SUPPORT_BASICS_H
#include <vector>
#include "Exception.h"
#include "debug.h"
#include <stdint.h>

namespace V3DLib {

/**
 * Terminate the application ASAP
 *
 * This happens by throwing an exception, rather than abruptly closing with `exit()` or `abort()`.
 * This allows the stack to unwind and all objects clean up behind them, also the global objects.
 *
 * This is significant for Buffer Objects, which are system-global resource. If these are not properly
 * deallocated, eventually the memory will clog up and the GPU can't be used any more.
 * This is an issue notably with vc4. The v3d has a higher tolerance level, but the issue is real there
 * as well.
 */
inline void fatal(const char *msg) {
  std::string str = "FATAL: ";
  str += msg;
  throw Exception(str);
}


inline void fatal(std::string const &msg) {
  fatal(msg.c_str());
}

}  // V3DLib


void findAndReplaceAll(std::string &data, std::string toSearch, std::string replaceStr);

std::string tabbed(int tab_size, std::string const &val);
std::string tabbed(int tab_size, int val);
inline std::string tabs(int tab_size) { return tabbed(tab_size, ""); }

std::string title(std::string const &str);

//
// Convenience definitions
//

template<typename T>
inline std::vector<T> &operator<<(std::vector<T> &a, T val) {
  a.push_back(val);  
  return a;
}


template<typename T>
inline std::vector<T> &operator<<(std::vector<T> &a, std::vector<T> const &b) {
  a.insert(a.end(), b.begin(), b.end());
  return a;
}


inline std::vector<std::string> &operator<<(std::vector<std::string> &a, char const *str) {
  a.push_back(str);  
  return a;
}


inline std::string &operator<<(std::string &a, char const *str)        { a += str; return a; }
inline std::string &operator<<(std::string &a, std::string const &str) { a += str; return a; }


inline std::string &operator<<(std::string &a, int val)      { a += std::to_string(val); return a; }
inline std::string &operator<<(std::string &a, long val)     { a += std::to_string(val); return a; }
inline std::string &operator<<(std::string &a, uint32_t val) { a += std::to_string(val); return a; }
inline std::string &operator<<(std::string &a, uint64_t val) { a += std::to_string(val); return a; }
inline std::string &operator<<(std::string &a, float val)    { a += std::to_string(val); return a; }
inline std::string &operator<<(std::string &a, bool val)     { a += val?"true":"false";  return a; }

#endif  // _V3DLIB_SUPPORT_BASICS_H
