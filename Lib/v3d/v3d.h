#ifndef _V3D_V3D_h
#define _V3D_V3D_h
#include <stdint.h>
#include <vector>
//#include "Common/SharedArray.h"

struct WorkGroup {
  uint32_t wg_x = 0;
  uint32_t wg_y = 0;
  uint32_t wg_z = 0;

  WorkGroup(uint32_t x = 16, uint32_t y = 1, uint32_t z = 1) :
    wg_x(x),
    wg_y(y),
    wg_z(z) {}

  uint32_t wg_size() { return wg_x * wg_y * wg_z; }
};


struct st_v3d_submit_csd {
  uint32_t cfg[7];
  uint32_t coef[4];
  uint64_t bo_handles;
  uint32_t bo_handle_count;
  uint32_t in_sync;
  uint32_t out_sync;
};

bool v3d_open();
bool v3d_close();
bool v3d_alloc(uint32_t size, uint32_t &handle, uint32_t &phyaddr, void **usraddr);
bool v3d_unmap(uint32_t size, uint32_t handle, void *usraddr);
bool v3d_wait_bo(std::vector<uint32_t> const &bo_handles, uint64_t timeout_ns);
int v3d_submit_csd(st_v3d_submit_csd &st);

#endif  // _V3D_V3D_h
