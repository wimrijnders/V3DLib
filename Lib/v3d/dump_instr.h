#ifndef _V3D_INSTR_DUMP_H
#define _V3D_INSTR_DUMP_H
#include "broadcom/common/v3d_device_info.h"  // v3d_qpu_instr_unpack() for calling programs
#include "broadcom/qpu/qpu_instr.h"

void instr_dump(char *buffer, struct v3d_qpu_instr *instr);

#endif  // _V3D_INSTR_DUMP_H
