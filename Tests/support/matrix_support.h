#ifndef _TEST_SUPPORT_MATRIX_SUPPORT_H
#define _TEST_SUPPORT_MATRIX_SUPPORT_H
#include <vector>
#include "Source/Float.h"
#include "Source/Complex.h"
#include "Support/pgm.h"

// ============================================================================
// Support routines for arrays and matrices in unit tests
// ============================================================================

void compare_arrays(V3DLib::Float::Array2D &a, float const *b, float precision = -1.0f);

inline void compare_arrays(V3DLib::Float::Array2D &a, std::vector<float> const &b, float precision = -1.0f) {
  compare_arrays(a, b.data(), precision);
}

void compare_arrays(V3DLib::Float::Array2D &a, V3DLib::Float::Array2D &b, float precision = -1.0f);

void compare_arrays(V3DLib::Complex::Array2D &a, V3DLib::Complex::Array2D &b, float precision = -1.0f);
void compare_arrays(std::vector<float> &a, float const *b);

void compare_array_scalar(V3DLib::Float::Array2D &arr, float scalar);

void check_unitary(std::vector<float> &a, int dim);
void check_unitary(V3DLib::Float::Array2D &a);

void fill_random(float *arr, int size);
void fill_random(std::vector<float> &arr);

void copy_array(V3DLib::Float::Array2D &dst, float const *src);
void copy_array(V3DLib::Float::Array2D &dst, std::vector<float> const &src);
void copy_transposed(float *dst, float const *src, int rows, int columns);
void copy_transposed(std::vector<float> &dst, std::vector<float> const &src, int rows, int columns);

#endif  // _TEST_SUPPORT_MATRIX_SUPPORT_H
