#ifndef _VC6_DRM_V3D_H_
#define _VC6_DRM_V3D_H_
#include "drm_types.h"


namespace QPULib {
namespace v3d {

class DRM_V3D {
public:
	void v3d_wait_bo(uint32_t handle, uint64_t timeout_ns);
	void v3d_submit_csd( st_v3d_submit_csd &st);

private:
	//void init();
	void done();
};

}  // v3d
}  // QPULib

#endif // _VC6_DRM_V3D_H_
