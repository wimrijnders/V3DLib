#ifndef _TEST_SUPPORT_SUMMATION_KERNEL_H
#define _TEST_SUPPORT_SUMMATION_KERNEL_H
#include <vector>
#include "support.h"

extern std::vector<uint64_t> summation; 

ByteCode summation_kernel(uint8_t num_qpus, int unroll_shift, int code_offset = 0);
void run_summation_kernel(std::vector<uint64_t> &bytecode, uint8_t num_qpus, int unroll_shift);

#endif  // _TEST_SUPPORT_SUMMATION_KERNEL_H
