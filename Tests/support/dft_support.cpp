#include "dft_support.h"
#include <iostream>
#include "Support/debug.h"
#include "Source/Functions.h"
#include "Support/Helpers.h"  // random_float()

/**
 * Wavelet function for testing
 *
 * The function is fairly arbitrary and tweakable.
 * Its goal is to produce distinct peaks in the transform,
 * despite noise and such.
 *
 * Note that it uses the son/cos approximation of V3DLib.
 */
float wavelet_function(int c, int const Dim) {
  using namespace V3DLib;
  assert(c < Dim);

  float offset = 0.1f;

  float freq_filter =  0.5f / ((float) Dim);
  float freq1       =  1.0f / ((float) Dim);
  float freq2       = 45.0f / ((float) Dim);

  float filter = functions::sin(freq_filter*((float) c), true);
  float noise = 0.3f *random_float();
  float val1  = 1.0f *functions::sin(freq1*((float) c), true);
  float val2  = 0.75f*functions::sin(freq2*((float) c), true);

  return (offset + noise + (filter*filter)*(val1 + val2));
}


void scalar_dump(cx *b, int size) {
  std::cout << "Scalar output: ";
  for (int i=0; i < size; ++i) 
    std::cout << b[i] << ", ";
  std::cout << std::endl;
}
