#ifndef _VC6_DRM_V3D_H_
#define _VC6_DRM_V3D_H_
#include "drm_types.h"


namespace QPULib {
namespace vc6 {

class DRM_V3D {
public:
	void v3d_wait_bo(uint32_t bo_handle, int timeout);

	void v3d_submit_csd(
		Cfg &cfg,
		Coef &coef,
		BoHandles bo_handles,
		uint32_t bo_handle_count,
		uint32_t in_sync,
		uint32_t out_sync);

private:
	int m_fd = -1;

	void init(int card = 0);
	void done();
};

}  // vc6
}  // QPULib

#endif // _VC6_DRM_V3D_H_
