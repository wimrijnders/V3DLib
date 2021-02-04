//
// Kernels for use with the 'Rot3D' example.
//
// These are kept in a separate file so that they can also be used
// in the unit tests.
//
// ============================================================================
#include "Rot3D.h"
#include "Source/Functions.h"

namespace kernels { 

using namespace V3DLib;


// ============================================================================
// Scalar version
// ============================================================================

void rot3D(int n, float cosTheta, float sinTheta, float* x, float* y) {
  for (int i = 0; i < n; i++) {
    float xOld = x[i];
    float yOld = y[i];
    x[i] = xOld * cosTheta - yOld * sinTheta;
    y[i] = yOld * cosTheta + xOld * sinTheta;
  }
}


// ============================================================================
// Kernel version 1
// ============================================================================

void rot3D_1(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y) {
  For (Int i = 0, i < n, i = i+16)
    Float xOld = x[i];
    Float yOld = y[i];
    x[i] = xOld * cosTheta - yOld * sinTheta;
    y[i] = yOld * cosTheta + xOld * sinTheta;
  End
}


// ============================================================================
// Kernel version 2
// ============================================================================

void rot3D_2(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y) {
  Int inc = numQPUs() << 4;
  Ptr<Float> p = x;
  Ptr<Float> q = y;
  gather(p); gather(q);
 
  Float xOld, yOld;
  For (Int i = 0, i < n, i = i+inc)
    gather(p+inc); gather(q+inc); 
    receive(xOld); receive(yOld);
    *p = xOld * cosTheta - yOld * sinTheta;
    *q = yOld * cosTheta + xOld * sinTheta;
    p = p+inc; q = q+inc;
  End

  receive(xOld); receive(yOld);
}


// ============================================================================
// Kernel version 3
// ============================================================================

namespace {
  int N = -1;       // Number of elements in incoming arrays for rot3D_3
  int numQPUs = -1; // Number of QPUs to use for rot3D_3
}  // anon namespace


void rot3D_3(Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y) {
  assert(N != -1);
  assert(numQPUs != -1);
  assertq(N % (16*numQPUs) == 0, "N must be a multiple of '16*numQPUs'");
  using V3DLib::functions::operator/;

  int size = N/numQPUs;
  int count = size/16;

  Ptr<Float> p_src = x - me()*16 + me()*size;
  Ptr<Float> q_src = y - me()*16 + me()*size;
  Ptr<Float> p_dst = x - me()*16 + me()*size;
  Ptr<Float> q_dst = y - me()*16 + me()*size;

  prefetch();
 
  Float xOld, yOld;
  // Can't do this here! For (Int i = 0, i < count, i++)
  // TODO register why (you know this)
  for (int i = 0; i < count; i++) {
    prefetch(xOld, p_src);
    prefetch(yOld, q_src);

    *p_dst = xOld * cosTheta - yOld * sinTheta;
    *q_dst = yOld * cosTheta + xOld * sinTheta;

    p_dst = p_dst+16;
    q_dst = q_dst+16;
  //End
  }
}


/**
 * Using decorator to avoid `N/numQPUs()` in source language code.
 * TODO retest to see effect of that division, optimize it if a problem. 
 */
FuncType *rot3D_3_decorator(int dimension, int in_numQPUs) {
  assert(dimension > 0);
  assertq(dimension % 16 == 0, "dimension must be a multiple of 16");
  // TODO perhaps assert in_numQPUs as well

  N = dimension;
  numQPUs = in_numQPUs;
  return rot3D_3;
}

}  // namespace kernels
