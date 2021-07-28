#ifndef _V3DLIB_KERNELS_DOTVECTOR_H_
#define _V3DLIB_KERNELS_DOTVECTOR_H_
#include "V3DLib.h"

namespace kernels {

using namespace V3DLib;

/**
 * Kernel helper class for loading in a sequence of values into QPU registers
 *
 * A Number of registers in the register file are allocated for the sequence.
 * These registers are indexed to retain the order.
 * 16 consequent values are loaded into the vector of a register.
 *
 * The goal here is to have the entire sequence of values loaded into the QPU
 * register file, so that it can be reused.
 * This, of course, places an upper limit on the length of the sequence.
 */
class DotVector {
public:
  DotVector(int size);

  void load(Float::Ptr input);
  void save(Float::Ptr dst);
  void dot_product(Float::Ptr rhs, Float &result);
  void dft_dot_product(Int const &row, Complex &result, int num_elements, Int const &offset = 0);
  size_t size() const { return elements.size(); }
  Float &operator[] (int index) { return elements[index]; }
  Float const &operator[] (int index) const { return elements[index]; }

private:
  std::vector<Float> elements;
};

void pre_write(Float::Ptr &dst, Float &src, bool add_result);

}  // namespace kernels


#endif  // _V3DLIB_KERNELS_DOTVECTOR_H_
