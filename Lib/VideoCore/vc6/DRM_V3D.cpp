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


namespace {
	const char *CARD_0  =  "/dev/dri/card0";
	const char *CARD_1  =  "/dev/dri/card0";

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

} // anon namespace


namespace QPULib {
namespace vc6 {

void DRM_V3D::init(int card) {
	assert(card == 0 || card ==1);

	const char *path = (card == 0)? CARD_0: CARD_1;

	m_fd = open(path, O_RDWR);
	assert(m_fd >= 0);
}


void DRM_V3D::done() {
	if (m_fd != -1 ) {
		int ret = close(m_fd);
		assert(ret >= 0);
		m_fd == -1;
	}
}

}  // vc6
}  // QPULib
