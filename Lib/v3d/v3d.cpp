/**
 * Adjusted from: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
 */
#include "v3d.h"
#include <cassert>
#include <sys/ioctl.h>
#include <cstddef>    // NULL
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>   // exit()
#include <stdio.h>
#include <unistd.h>   // close()
#include "../debug.h"

namespace {

int fd = 0;

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


int submit_csd(
	int fd,
	uint32_t phyaddr,
	std::vector<uint32_t> const &bo_handles,
	uint32_t uniforms_address
) {
		assert(bo_handles.size() > 0);  // There should be at least one, for the code

    const uint32_t wg_x = 1;
    const uint32_t wg_y = 1;
    const uint32_t wg_z = 1;
    const uint32_t wg_size = wg_x * wg_y * wg_z;
    const uint32_t wgs_per_sg = 1;

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
    csd.cfg[6] = uniforms_address;  // allowed to be 0
    csd.coef[0] = 0;
    csd.coef[1] = 0;
    csd.coef[2] = 0;
    csd.coef[3] = 0;
    csd.bo_handles = (uintptr_t) bo_handles.data();
    csd.bo_handle_count = bo_handles.size();
    csd.in_sync = 0;
    csd.out_sync = 0;

	int ret =  ioctl(fd, IOCTL_V3D_SUBMIT_CSD, &csd);  // !!!!! pass a pointer! (IDIOT)
	if (ret) {
		perror(NULL);
		assert(false);
	}

	return ret;
}





bool alloc_intern(int fd, uint32_t size, uint32_t &handle, uint32_t &phyaddr, void **usraddr) {
		assert(fd != 0);

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
    handle  = create_bo.handle;
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

int open_card(char const *card) {
	int fd = open(card , O_RDWR);

	if (fd == 0) {
		printf("FATAL: Can't open card device (sudo?)\n");
		exit(-1);
	}

	//
	// Perform an operation on the device: allocate 16 bytes of memory.
	// The 'wrong' card will fail here
	//
	{
		const uint32_t ALLOC_SIZE = 16;

		// Place a call on it see if it works
    uint32_t handle = 0;
    uint32_t phyaddr = 0;
		void *usraddr = nullptr;

		if (alloc_intern(fd, ALLOC_SIZE, handle, phyaddr, &usraddr)) {
			// worked! clean up
			v3d_unmap(ALLOC_SIZE, handle, usraddr);
		} else {
			// fail
			fd = 0;
		}
	}

	return fd;
}

}  // anon namespace



/**
 * Apparently, you don't need to close afterwards.
 * If you try, then you get the perror:
 *
 *    Inappropriate ioctl for device
 */
bool v3d_open() {
	if (fd != 0) {
		// Already open, all is well
		return true;
	}

	// It appears to be a random crap shoot which device card0 and card1 address
	// So we try both, test them and pick the one that works (if any)
	int fd0 = open_card("/dev/dri/card0");
	int fd1 = open_card("/dev/dri/card1");

	if (fd0 == 0) {
		assert(fd1 != 0);
		fd = fd1;
	} else if (fd1 == 0) {
		assert(fd0 != 0);
		fd = fd0;
	} else {
		printf("FATAL: could not open card device\n");
		exit(-1);
	}

	return (fd > 0);
}

void v3d_close() {
	if (fd != -1 ) {
		int ret = close(fd);
		assert(ret >= 0);
		fd = 0;
	}
}


int v3d_fd() {
	if (!v3d_open()) {
		assert(false);
	}

	return fd;
}


bool v3d_alloc(uint32_t size, uint32_t &handle, uint32_t &phyaddr, void **usraddr) {
		assert(size > 0);
		assert(handle == 0);
		assert(phyaddr == 0);
		assert(*usraddr == nullptr);

	return  alloc_intern(fd, size, handle, phyaddr, usraddr);
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


int v3d_wait_bo(int fd, uint32_t handle) {
    drm_v3d_wait_bo wait;
    wait.handle = handle;
    wait.pad = 0;
    wait.timeout_ns = 10llu * 1000000000llu;
    int ret = ioctl(fd, IOCTL_V3D_WAIT_BO, &wait);

	if(ret) {
		perror(NULL);
		assert(false);
	}

	return ret;
}
