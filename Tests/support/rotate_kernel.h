#ifndef _TEST_SUPPORT_ROTATE_KERNEL_H
#define _TEST_SUPPORT_ROTATE_KERNEL_H
#include <stdint.h>
#include <vector>
#include "support.h"

extern V3DLib::v3d::ByteCode const qpu_rotate_alias_code;
V3DLib::v3d::ByteCode rotate_kernel();
void run_rotate_alias_kernel(V3DLib::v3d::ByteCode const &bytecode);

#endif  // _TEST_SUPPORT_ROTATE_KERNEL_H
