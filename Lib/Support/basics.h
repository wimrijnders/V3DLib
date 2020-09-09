#ifndef _LIB_SUPPORT_BASICS_h
#define _LIB_SUPPORT_BASICS_h
#include <string>
#include <vector>
#include "debug.h"

namespace QPULib {

struct Exception : public std::exception {
	Exception(std::string ss) : s(ss) {}
	~Exception() throw () {}

	const char *what() const throw() { return s.c_str(); }

private:
	std::string s;
};


/**
 * Terminate the application ASAP
 *
 * This happens by throwing an exception, rather than abruptly closing with `exit()` or `abort()`.
 * This allows the stack to unwind and all objects clean up behind them, also the global objects.
 *
 * This is significant for Buffer Objects, which are system-global resource. If these are not properly
 * deallocated, eventually the memory will clog up and the GPU can't be used any more.
 * This is an issue notably with vc4. The v3d has a higher tolerance level, but the issue is real there
 * as well (TODO research, it might be smart enough to clean up on program exit).
 */
inline void fatal(const char *msg) {
	std::string str = "FATAL: ";
	str += msg;
	throw Exception(str);
}

}  // QPULib


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

#endif  // _LIB_SUPPORT_BASICS_h
