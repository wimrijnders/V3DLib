#ifndef _TEST_SUMMATION_h
#define _TEST_SUMMATION_h
#include <vector>
#include <stdint.h>

extern std::vector<uint64_t> summation; 

std::vector<uint64_t> summation_kernel(uint8_t num_qpus);
#endif  // _TEST_SUMMATION_h
