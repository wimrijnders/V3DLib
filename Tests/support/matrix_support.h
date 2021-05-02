#ifndef _TEST_SUPPORT_MATRIX_SUPPORT_H
#define _TEST_SUPPORT_MATRIX_SUPPORT_H
#include <vector>
#include "Source/Float.h"
#include "Source/Complex.h"
#include "Support/pgm.h"

void compare_arrays(V3DLib::Float::Array2D &a, float const *b);

inline void compare_arrays(V3DLib::Float::Array2D &a, std::vector<float> const &b) {
  compare_arrays(a, b.data());
}

void compare_arrays(V3DLib::Float::Array2D &a, V3DLib::Float::Array2D &b, float precision = -1.0f);

void compare_arrays(V3DLib::Complex::Array2D &a, V3DLib::Complex::Array2D &b, float precision = -1.0f);
void compare_arrays(std::vector<float> &a, float const *b);

void check_unitary(std::vector<float> &a, int dim);
void check_unitary(V3DLib::Float::Array2D &a);

#endif  // _TEST_SUPPORT_MATRIX_SUPPORT_H
