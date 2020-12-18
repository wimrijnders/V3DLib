#ifndef _TEST_SUPPORT_SUMMATION_KERNEL_H
#define _TEST_SUPPORT_SUMMATION_KERNEL_H
#include <vector>
#include "support.h"

extern std::vector<uint64_t> summation; 

V3DLib::v3d::ByteCode summation_kernel(uint8_t num_qpus, int unroll_shift, int code_offset = 0);
void run_summation_kernel(V3DLib::v3d::ByteCode &bytecode, uint8_t num_qpus, int unroll_shift);

#endif  // _TEST_SUPPORT_SUMMATION_KERNEL_H
