//
// Placeholder definitions for certain data types.
//
// These should be filled out in due time
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VC6_DRM_TYPES_H_
#define _VC6_DRM_TYPES_H_
#include <sys/ioctl.h>
#include <stdint.h>
#include <array>
#include <vector>
#include "../Target/SharedArray.h"


namespace QPULib {
namespace v3d {

using BoHandles = std::vector<uint32_t>;

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
		//Cfg    cfg; // c_uint32 * 7
		//Coef   coef;  // c_uint32 * 4
		uint32_t cfg[7];
		uint32_t coef[4];
		uint64_t bo_handles;
		uint32_t bo_handle_count;
		uint32_t in_sync;
		uint32_t out_sync;
	};

}  // v3d
}  // QPULib


#endif  // _VC6_DRM_TYPES_H_
