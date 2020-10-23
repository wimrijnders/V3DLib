#ifndef _QPULIB_SMALL_LITERAL_H_
#define _QPULIB_SMALL_LITERAL_H_
#include "EmuSupport.h"  // Word

namespace QPULib {

class Expr;

int encodeSmallLit(Expr const &e);
std::string printSmallLit(int x);
Word decodeSmallLit(int x);

}  // namespace QPULib

#endif  // _QPULIB_SMALL_LITERAL_H_
