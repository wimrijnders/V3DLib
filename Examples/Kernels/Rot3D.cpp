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

void rot3D_3(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y) {
  using V3DLib::functions::operator/;

  Int inc = numQPUs() << 4;
  Int size = n/numQPUs();

  Ptr<Float> p_src = x - me()*inc + me()*size;
  Ptr<Float> q_src = y - me()*inc + me()*size;
  Ptr<Float> p_dst = x - me()*inc + me()*size;
  Ptr<Float> q_dst = y - me()*inc + me()*size;

  gather(p_src); gather(q_src);
 
  Float xOld, yOld;
  For (Int i = 0, i < size, i++)
    gather(p_src+16); gather(q_src+16); 
    receive(xOld); receive(yOld);
    *p_dst = xOld * cosTheta - yOld * sinTheta;
    *q_dst = yOld * cosTheta + xOld * sinTheta;
    p_src = p_src+16; q_src = q_src+16;
    p_dst = p_dst+16; q_dst = q_dst+16;
  End

  receive(xOld); receive(yOld);
}

}  // namespace kernels
