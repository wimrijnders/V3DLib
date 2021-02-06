#ifndef _LIB_SUPPORT_EXCEPTION_H
#define _LIB_SUPPORT_EXCEPTION_H
#include <string>

namespace V3DLib {

struct Exception : public std::exception {
  Exception(std::string ss) : s(ss) {}
  ~Exception() throw () {}

  const char *what() const throw() override { return s.c_str(); }
  std::string const &msg() const throw() { return s; }

private:
  std::string s;
};

}  // namespace V3DLib

#endif  // _LIB_SUPPORT_EXCEPTION_H
