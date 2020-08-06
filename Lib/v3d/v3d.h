#ifndef _V3D_V3D_h
#define _V3D_V3D_h
#include <stdint.h>
#include "SharedArray.h"


bool v3d_submit_csd(uint32_t phyaddr, uint32_t handle);
inline bool v3d_submit_csd(QPULib::v3d::SharedArrayBase const &codeMem) {
	return v3d_submit_csd(codeMem.getPhyAddr(), codeMem.getHandle());
}

bool v3d_open();
void v3d_close();
int v3d_fd();
bool v3d_alloc(uint32_t size, uint32_t &handle, uint32_t &phyaddr, void **usraddr);
bool v3d_unmap(uint32_t size, uint32_t handle, void *usraddr);


#endif  // _V3D_V3D_h
