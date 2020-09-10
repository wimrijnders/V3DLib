#ifndef _TEST_SUPPORT_ROTATE_KERNEL_H
#define _TEST_SUPPORT_ROTATE_KERNEL_H
#include <stdint.h>
#include <vector>
#include "support.h"

extern const std::vector<uint64_t> qpu_rotate_alias_code;
ByteCode rotate_kernel();

#endif  // _TEST_SUPPORT_ROTATE_KERNEL_H
