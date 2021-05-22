#ifndef _TEST_SUPPORT_DFT_SUPPORT_H
#define _TEST_SUPPORT_DFT_SUPPORT_H
#include <complex>
#include "Source/Complex.h"

using cx = std::complex<double>;

inline cx operator-(cx const &a, V3DLib::complex const &b) { return cx(a.real() - b.re(), a.imag() - b.im()); }
//cx operator-(complex const &a, V3DLib::cx const &b) { return cx(a.re() - b.real(), a.im() - b.imag()); }

float wavelet_function(int c, int const Dim);

void scalar_dump(cx *b, int size);

#endif  // _TEST_SUPPORT_DFT_SUPPORT_H
