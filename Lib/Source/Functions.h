#ifndef _V3DLIB_SOURCE_FUNCTIONS_H_
#define _V3DLIB_SOURCE_FUNCTIONS_H_
#include "Int.h"

namespace V3DLib {
namespace functions {

IntExpr two_complement(IntExpr a);
IntExpr abs(IntExpr a);
IntExpr operator/(IntExpr in_a, IntExpr in_b);

inline IntExpr operator-(IntExpr a) { return two_complement(a); }

}  // namespace functions
}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_FUNCTIONS_H_
