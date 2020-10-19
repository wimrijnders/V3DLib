#ifndef _LIB_SUPPORT_EXCEPTION_H
#define _LIB_SUPPORT_EXCEPTION_H
#include <string>

namespace QPULib {

struct Exception : public std::exception {
	Exception(std::string ss) : s(ss) {}
	~Exception() throw () {}

	const char *what() const throw() override { return s.c_str(); }
	std::string const &msg() const throw() { return s; }

private:
	std::string s;
};

}  // namespace QPULib

#endif  // _LIB_SUPPORT_EXCEPTION_H
