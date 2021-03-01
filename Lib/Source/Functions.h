#ifndef _V3DLIB_SOURCE_FUNCTIONS_H_
#define _V3DLIB_SOURCE_FUNCTIONS_H_
#include "Int.h"
#include "Float.h"

namespace V3DLib {
namespace functions {

IntExpr two_complement(IntExpr a);
IntExpr abs(IntExpr a);
IntExpr operator/(IntExpr in_a, IntExpr in_b);

inline IntExpr operator-(IntExpr a) { return two_complement(a); }

float cos(float x_in, bool extra_precision = false) noexcept;
float sin(float x_in, bool extra_precision = false) noexcept;

FloatExpr ffloor(FloatExpr x);
FloatExpr fabs(FloatExpr x);

}  // namespace functions

void rotate_sum(Int &input, Int &result);
void rotate_sum(Float &input, Float &result);
void set_at(Int &dst, Int n, Int const &src);
void set_at(Float &dst, Int n, Float const &src);


}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_FUNCTIONS_H_
