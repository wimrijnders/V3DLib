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

struct st_v3d_wait_bo {
	uint32_t handle;
	uint32_t pad;
	uint64_t timeout_ns;
};

// Derived from linux/include/uapi/drm/drm.h
#define DRM_IOCTL_BASE   'd'
#define DRM_COMMAND_BASE 0x40
#define DRM_GEM_CLOSE    0x09

// Derived from linux/include/uapi/drm/v3d_drm.h
#define DRM_V3D_WAIT_BO    (DRM_COMMAND_BASE + 0x01)
#define DRM_V3D_CREATE_BO  (DRM_COMMAND_BASE + 0x02)
#define DRM_V3D_MMAP_BO    (DRM_COMMAND_BASE + 0x03)
#define DRM_V3D_GET_PARAM  (DRM_COMMAND_BASE + 0x04)
#define DRM_V3D_SUBMIT_CSD (DRM_COMMAND_BASE + 0x07)

#define IOCTL_GEM_CLOSE      _IOW(DRM_IOCTL_BASE, DRM_GEM_CLOSE, gem_close)
#define IOCTL_V3D_CREATE_BO  _IOWR(DRM_IOCTL_BASE, DRM_V3D_CREATE_BO, drm_v3d_create_bo)
#define IOCTL_V3D_MMAP_BO    _IOWR(DRM_IOCTL_BASE, DRM_V3D_MMAP_BO, drm_v3d_mmap_bo)
#define IOCTL_V3D_WAIT_BO    _IOWR(DRM_IOCTL_BASE, DRM_V3D_WAIT_BO, st_v3d_wait_bo)
#define IOCTL_V3D_SUBMIT_CSD _IOW(DRM_IOCTL_BASE, DRM_V3D_SUBMIT_CSD, st_v3d_submit_csd)

const unsigned V3D_PARAM_V3D_UIFCFG = 0;
const unsigned V3D_PARAM_V3D_HUB_IDENT1 = 1;
const unsigned V3D_PARAM_V3D_HUB_IDENT2 = 2;
const unsigned V3D_PARAM_V3D_HUB_IDENT3 = 3;
const unsigned V3D_PARAM_V3D_CORE0_IDENT0 = 4;
const unsigned V3D_PARAM_V3D_CORE0_IDENT1 = 5;
const unsigned V3D_PARAM_V3D_CORE0_IDENT2 = 6;
const unsigned V3D_PARAM_SUPPORTS_TFU = 7;
const unsigned V3D_PARAM_SUPPORTS_CSD = 8;


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


void fd_close(int fd) {
	if (fd > 0 ) {
		int ret = close(fd);
		assert(ret >= 0);
	}
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
			printf("open_card(): alloc test succeeded for card %s\n", card);
		} else {
			// fail
			fd_close(fd);
			fd = 0;
			printf("open_card(): alloc test FAILED for card %s\n", card);
		}
	}

	return fd;
}

}  // anon namespace


/**
 *
 */
int v3d_submit_csd(st_v3d_submit_csd &st) {
	int ret = ioctl(fd, IOCTL_V3D_SUBMIT_CSD, &st);

	if(ret) {
		perror(NULL);
		assert(false);
	}

	return ret;
}


/**
 * Apparently, you don't need to close afterwards.
 * If you try, then you get the perror:
 *
 *    Inappropriate ioctl for device
 *
 * @return true if opening succeeded, false otherwise
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

	if (fd > 0) {
		printf("v3d_open() succeeded\n");
	}

	return (fd > 0);
}


void v3d_close() {
	fd_close(fd);
	fd = 0;
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


int v3d_wait_bo(uint32_t handle, uint64_t timeout_ns) {
	assert(handle != 0);
	assert(timeout_ns > 0);

	st_v3d_wait_bo st = {
		handle,
		0,
		timeout_ns,
	};

	int ret = ioctl(fd, IOCTL_V3D_WAIT_BO, &st);
	if (ret) {
		perror(NULL);
		assert(false);
	}

	return ret;
}


/**
 * @return true if wait succeeded, false otherwise
 */
bool v3d_wait_bo(std::vector<uint32_t> const &bo_handles, uint64_t timeout_ns) {
	assert(bo_handles.size() > 0);
	assert(timeout_ns > 0);

	int ret = true;

	for (auto handle : bo_handles) {
		int result = v3d_wait_bo(handle, timeout_ns);
		if (0 != result) {
			printf("v3d_wait_bo() returned %d\n", ret);
			ret = false;
		}
	}
	//printf("Done calling v3d_wait_bo()\n");

	return ret;
}
