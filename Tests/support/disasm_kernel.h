#ifndef  _TESTS_SUPPORT_QPU_DISASM_KERNEL_H_
#define  _TESTS_SUPPORT_QPU_DISASM_KERNEL_H_
#include <stdint.h>
#include <vector>

std::vector<uint64_t> &qpu_disasm_bytecode();
std::vector<uint64_t> qpu_disasm_kernel();

#endif  // _TESTS_SUPPORT_QPU_DISASM_KERNEL_H_
