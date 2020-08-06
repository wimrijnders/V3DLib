// 
// Converted from: https://github.com/Idein/py-videocore6/blob/c571a6b507e0f19ada0d50620e670eab424fc2a8/videocore6/drm_v3d.py
//
///////////////////////////////////////////////////////////////////////////////
#include <cassert>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>   // open()
#include <unistd.h>  // close()
#include <stdint.h>
#include "DRM_V3D.h"
#include "v3d.h"
#include "debug.h"


namespace {
	using QPULib::v3d::Cfg;
	using QPULib::v3d::Coef;

	const char *CARD_0  =  "/dev/dri/card0";
	const char *CARD_1  =  "/dev/dri/card1";

		// Derived from linux/include/uapi/drm/drm.h
    const unsigned DRM_IOCTL_BASE = 'd';
    const unsigned DRM_COMMAND_BASE = 0x40;
    const unsigned DRM_GEM_CLOSE = 0x09;

    // Derived from linux/include/uapi/drm/v3d_drm.h
    const unsigned DRM_V3D_WAIT_BO = DRM_COMMAND_BASE + 0x01;
    const unsigned DRM_V3D_CREATE_BO = DRM_COMMAND_BASE + 0x02;
    const unsigned DRM_V3D_MMAP_BO = DRM_COMMAND_BASE + 0x03;
    const unsigned DRM_V3D_GET_PARAM = DRM_COMMAND_BASE + 0x04;
    const unsigned DRM_V3D_SUBMIT_CSD = DRM_COMMAND_BASE + 0x07;

    const unsigned V3D_PARAM_V3D_UIFCFG = 0;
    const unsigned V3D_PARAM_V3D_HUB_IDENT1 = 1;
    const unsigned V3D_PARAM_V3D_HUB_IDENT2 = 2;
    const unsigned V3D_PARAM_V3D_HUB_IDENT3 = 3;
    const unsigned V3D_PARAM_V3D_CORE0_IDENT0 = 4;
    const unsigned V3D_PARAM_V3D_CORE0_IDENT1 = 5;
    const unsigned V3D_PARAM_V3D_CORE0_IDENT2 = 6;
    const unsigned V3D_PARAM_SUPPORTS_TFU = 7;
    const unsigned V3D_PARAM_SUPPORTS_CSD = 8;

struct st_v3d_wait_bo {
	uint32_t handle;
	uint32_t pad;
	uint64_t timeout_ns;
};

	struct st_v3d_submit_csd {
		Cfg    cfg; // c_uint32 * 7
		Coef   coef;  // c_uint32 * 4
		uint64_t bo_handles;
		uint32_t bo_handle_count;
		uint32_t in_sync;
		uint32_t out_sync;
	};



	const unsigned IOCTL_V3D_WAIT_BO    = _IOWR(DRM_IOCTL_BASE, DRM_V3D_WAIT_BO, sizeof(st_v3d_wait_bo));
	const unsigned IOCTL_V3D_SUBMIT_CSD = _IOW(DRM_IOCTL_BASE, DRM_V3D_SUBMIT_CSD, sizeof(st_v3d_submit_csd));

} // anon namespace


namespace QPULib {
namespace v3d {

void DRM_V3D::v3d_wait_bo(uint32_t handle, uint64_t timeout_ns) {
	//assert(false);  // TODO

	st_v3d_wait_bo st = {
		handle,
		0,
		timeout_ns,
	};

	if(ioctl(v3d_fd(), IOCTL_V3D_WAIT_BO, st)) {
		perror(NULL);
		assert(false);
	}
}


/**
 * TODO: There also a submit_csd() in `v3d.cpp`, consolidate
 */
void DRM_V3D::v3d_submit_csd(
	Cfg &cfg,
	Coef &coef,
	BoHandles &bo_handles,
	uint32_t in_sync,
	uint32_t out_sync) {

	st_v3d_submit_csd st = {
		// XXX: Dirty hack!
		cfg,
		coef,
		(uint64_t) bo_handles.data(),
		bo_handles.size(),
		in_sync,
		out_sync
	};

	if(ioctl(v3d_fd(), IOCTL_V3D_SUBMIT_CSD, st)) {
		perror(NULL);
		assert(false);
	}
}

}  // v3d
}  // QPULib
