/**
 * Adjusted from: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
 */
#include "v3d.h"
#include <cassert>
#include <sys/ioctl.h>
#include <cstddef>  // NULL
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>  // close
#include "../../debug.h"

namespace {

typedef struct {
    uint32_t size;
    uint32_t flags;
    uint32_t handle;
    uint32_t offset;
} drm_v3d_create_bo;

typedef struct {
    uint32_t handle;
    uint32_t flags;
    uint64_t offset;
} drm_v3d_mmap_bo;

typedef struct {
    uint32_t  handle;
    uint32_t  pad;
} gem_close;

typedef struct {
    uint32_t handle;
    uint32_t pad;
    uint64_t timeout_ns;
} drm_v3d_wait_bo;

typedef struct {
    uint32_t cfg[7];
    uint32_t coef[4];
    uint64_t bo_handles;
    uint32_t bo_handle_count;
    uint32_t in_sync;
    uint32_t out_sync;
} drm_v3d_submit_csd;

#define DRM_IOCTL_BASE   'd'
#define DRM_COMMAND_BASE 0x40
#define DRM_GEM_CLOSE    0x09

#define DRM_V3D_WAIT_BO    (DRM_COMMAND_BASE + 0x01)
#define DRM_V3D_CREATE_BO  (DRM_COMMAND_BASE + 0x02)
#define DRM_V3D_MMAP_BO    (DRM_COMMAND_BASE + 0x03)
#define DRM_V3D_WAIT_BO    (DRM_COMMAND_BASE + 0x01)
#define DRM_V3D_SUBMIT_CSD (DRM_COMMAND_BASE + 0x07)

#define IOCTL_GEM_CLOSE      _IOW(DRM_IOCTL_BASE, DRM_GEM_CLOSE, gem_close)
#define IOCTL_V3D_CREATE_BO  _IOWR(DRM_IOCTL_BASE, DRM_V3D_CREATE_BO, drm_v3d_create_bo)
#define IOCTL_V3D_MMAP_BO    _IOWR(DRM_IOCTL_BASE, DRM_V3D_MMAP_BO, drm_v3d_mmap_bo)
#define IOCTL_V3D_WAIT_BO    _IOWR(DRM_IOCTL_BASE, DRM_V3D_WAIT_BO, drm_v3d_wait_bo)
#define IOCTL_V3D_SUBMIT_CSD _IOW(DRM_IOCTL_BASE, DRM_V3D_SUBMIT_CSD, drm_v3d_submit_csd)


int submit_csd(int fd, uint32_t phyaddr, uint32_t handle) {
    const uint32_t wg_x = 1;
    const uint32_t wg_y = 1;
    const uint32_t wg_z = 1;
    const uint32_t wg_size = wg_x * wg_y * wg_z;
    const uint32_t wgs_per_sg = 1;
    const uint32_t bo_handles[] = { handle };
    drm_v3d_submit_csd csd;
    csd.cfg[0] = wg_x << 16;
    csd.cfg[1] = wg_y << 16;
    csd.cfg[2] = wg_z << 16;
    csd.cfg[3] =
        ((((wgs_per_sg * wg_size + 16u - 1u) / 16u) - 1u) << 12) |
        (wgs_per_sg << 8) |
        (wg_size & 0xff);
    csd.cfg[4] = 0;
    csd.cfg[5] = phyaddr;
    csd.cfg[6] = 0;
    csd.coef[0] = 0;
    csd.coef[1] = 0;
    csd.coef[2] = 0;
    csd.coef[3] = 0;
    csd.bo_handles = (uintptr_t)bo_handles;
    csd.bo_handle_count = sizeof(bo_handles)/sizeof(bo_handles[0]);
    csd.in_sync = 0;
    csd.out_sync = 0;
    return ioctl(fd, IOCTL_V3D_SUBMIT_CSD, &csd);
}


int wait_bo(int fd, uint32_t handle) {
    drm_v3d_wait_bo wait;
    wait.handle = handle;
    wait.pad = 0;
    wait.timeout_ns = 10e9;
    return ioctl(fd, IOCTL_V3D_WAIT_BO, &wait);
}

/////////////////////////////////////////////////////////////////////////

int fd = 0;

}  // anon namespace


/**
 * Apparently, you don't need to close afterwards.
 * If you try, then you get the perror:
 *
 *    Inappropriate ioctl for device
 */
bool v3d_open() {
	if (fd == 0) {
		// It appears to be a random crap shoot which device card0 and card1 address
		// TODO: Find a way to determine correct device entry
		fd = open("/dev/dri/card0", O_RDWR);
	}

	return (fd > 0);
}


bool v3d_alloc(uint32_t size, uint32_t &handle, uint32_t &phyaddr, void **usraddr) {
		assert(fd != 0);
		assert(size > 0);
		assert(handle == 0);
		assert(phyaddr == 0);
		assert(*usraddr == nullptr);

    drm_v3d_create_bo create_bo;
    create_bo.size = size;
    create_bo.flags = 0;
    {
        int res = ioctl(fd, IOCTL_V3D_CREATE_BO, &create_bo);
				if (res != 0) {
					perror(NULL);
					return false;
				}
    }
    handle = create_bo.handle;
    phyaddr = create_bo.offset;

    drm_v3d_mmap_bo mmap_bo;
    mmap_bo.handle = handle;
    mmap_bo.flags = 0;
    {
        int res = ioctl(fd, IOCTL_V3D_MMAP_BO, &mmap_bo);
				if (res != 0) {
					perror(NULL);
					return false;
				}
    }
    *usraddr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (__off_t) mmap_bo.offset);
    return (usraddr != MAP_FAILED);
}


bool v3d_unmap(uint32_t size, uint32_t handle,  void *usraddr) {
	assert(size > 0);
	assert(handle > 0);
	assert(usraddr != nullptr);

	int res = munmap(usraddr, size);
	if (res != 0) {
		return false;
	}

	gem_close cl;
	cl.handle = handle;
	return (ioctl(fd, IOCTL_GEM_CLOSE, &cl) == 0);
}


bool v3d_submit_csd(uint32_t phyaddr, uint32_t handle) {
	breakpoint

	assert(fd > 0);	

	if (submit_csd(fd, phyaddr, handle)) {
		perror(NULL);
		return false;
	}

	wait_bo(fd, handle);
	return true;
}

