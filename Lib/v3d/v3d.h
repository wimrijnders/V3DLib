#ifndef _V3D_V3D_h
#define _V3D_V3D_h
#include <stdint.h>

bool v3d_submit_csd(uint32_t phyaddr, uint32_t handle);
bool v3d_open();
bool v3d_alloc(uint32_t size, uint32_t &handle, uint32_t &phyaddr, void **usraddr);
bool v3d_unmap(uint32_t size, uint32_t handle, void *usraddr);

#endif  // _V3D_V3D_h
