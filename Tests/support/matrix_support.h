#ifndef _TEST_SUPPORT_MATRIX_SUPPORT_H
#define _TEST_SUPPORT_MATRIX_SUPPORT_H
#include "Source/Float.h"
#include "Source/Complex.h"
#include "Support/pgm.h"

void compare_arrays(V3DLib::Float::Array2D &a, float const *b);
void compare_arrays(V3DLib::Complex::Array2D &a, V3DLib::Complex::Array2D &b, float precision = -1.0f);

#endif  // _TEST_SUPPORT_MATRIX_SUPPORT_H
