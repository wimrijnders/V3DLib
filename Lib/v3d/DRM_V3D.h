#ifndef _VC6_DRM_V3D_H_
#define _VC6_DRM_V3D_H_
#include "drm_types.h"


namespace QPULib {
namespace v3d {

class DRM_V3D {
public:
	bool v3d_wait_bo(std::vector<uint32_t> const &bo_handles, uint64_t timeout_ns);
	int v3d_submit_csd( st_v3d_submit_csd &st);

private:
	//void init();
	void done();
};

}  // v3d
}  // QPULib

#endif // _VC6_DRM_V3D_H_
