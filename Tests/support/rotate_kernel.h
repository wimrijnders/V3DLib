#ifndef _TEST_SUPPORT_ROTATE_KERNEL_H
#define _TEST_SUPPORT_ROTATE_KERNEL_H
#include <stdint.h>
#include <vector>
#include "support.h"

extern ByteCode const qpu_rotate_alias_code;
ByteCode rotate_kernel(bool use_add_alu);
void run_rotate_alias_kernel(ByteCode const &bytecode);

#endif  // _TEST_SUPPORT_ROTATE_KERNEL_H
