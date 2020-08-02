#ifndef _V3D_INSTR_DUMP_H
#define _V3D_INSTR_DUMP_H
#include "broadcom/qpu/qpu_instr.h"
#include "broadcom/common/v3d_device_info.h"  // v3d_qpu_instr_unpack() for calling programs

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


void instr_dump(char *buffer, struct v3d_qpu_instr *instr);
bool instr_unpack(const struct v3d_device_info *devinfo, uint64_t packed_instr, struct v3d_qpu_instr *instr);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // _V3D_INSTR_DUMP_H
