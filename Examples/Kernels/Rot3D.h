#ifndef _EXAMPLES_KERNELS_ROT3D_H_
#define _EXAMPLES_KERNELS_ROT3D_H_
#include <V3DLib.h>

namespace kernels {

using namespace V3DLib;

void rot3D(int n, float cosTheta, float sinTheta, float* x, float* y);
void rot3D_1(Int n, Float cosTheta, Float sinTheta, Float::Ptr x, Float::Ptr y);
void rot3D_1a(Int n, Float cosTheta, Float sinTheta, Float::Ptr x, Float::Ptr y);
void rot3D_2(Int n, Float cosTheta, Float sinTheta, Float::Ptr x, Float::Ptr y);

void rot3D_3(Float cosTheta, Float sinTheta, Float::Ptr x, Float::Ptr y);

using FuncType = decltype(rot3D_3);

FuncType *rot3D_3_decorator(int dimension, int in_numQPUs = 1);

}  // namespace kernels

#endif  // _EXAMPLES_KERNELS_ROT3D_H_
