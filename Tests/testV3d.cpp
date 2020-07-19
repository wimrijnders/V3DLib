#include "catch.hpp"
#ifdef QPU_MODE
#include <cassert>
#include <array>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <cstring>
#include <sys/mman.h>  // TODO remove
#include <fcntl.h>
#include <stdint.h>
#include "VideoCore/vc6/SharedArray.h"


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

namespace {

//std::array<uint64_t, 8> do_nothing = {
uint64_t do_nothing[] = {
    0x3c203186bb800000, // nop; thrsw
    0x3c203186bb800000, // nop; thrsw
    0x3c003186bb800000, // nop
    0x3c003186bb800000, // nop
    0x3c203186bb800000, // nop; thrsw
    0x3c003186bb800000, // nop
    0x3c003186bb800000, // nop
    0x3c003186bb800000, // nop
};


static int submit_csd(int fd, uint32_t phyaddr, uint32_t handle) {
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

static int wait_bo(int fd, uint32_t handle) {
    drm_v3d_wait_bo wait;
    wait.handle = handle;
    wait.pad = 0;
    wait.timeout_ns = 10e9;
    return ioctl(fd, IOCTL_V3D_WAIT_BO, &wait);
}

static double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + t.tv_usec * 1e-6;
}

}  // anon namespace


/**
 * Adjusted from: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
 */
TEST_CASE("Check v3d code is working properly", "[v3d]") {
		// It appears to be a random crap shoot which device card0 and card1 address
		// TODO: Find a way to determine correct device entry
    int fd = open("/dev/dri/card1", O_RDWR);
    assert(fd > 0);

    drm_v3d_create_bo create_bo;
    create_bo.size = sizeof(do_nothing);
    create_bo.flags = 0;
    {
        int res = ioctl(fd, IOCTL_V3D_CREATE_BO, &create_bo);
				printf("res: %d\n", res);
				perror(NULL);
        assert(res == 0);
    }
    uint32_t handle = create_bo.handle;
    uint32_t phyaddr = create_bo.offset;

    drm_v3d_mmap_bo mmap_bo;
    mmap_bo.handle = handle;
    mmap_bo.flags = 0;
    {
        int res = ioctl(fd, IOCTL_V3D_MMAP_BO, &mmap_bo);
        assert(res == 0);
    }
    void* usraddr = mmap(NULL, sizeof(do_nothing), PROT_READ | PROT_WRITE, MAP_SHARED, fd, (uint32_t) mmap_bo.offset);
    assert(usraddr != MAP_FAILED);

    //memcpy(usraddr, do_nothing.data(), do_nothing.size());
    memcpy(usraddr, do_nothing, sizeof(do_nothing));


		QPULib::v3d::SharedArray<uint32_t> codeMem;

		//
		// NOTE: During testing, execution time shot up from 0.1 sec to 10 sec.
		//       This probably due to the gpu hanging because of previously job faulty (too short)
		//       The 10 sec is likely the timeout.
		//
		// TODO: Find a way to reset the v3d
		//
		printf("[submit]\n");
		double start = get_time();
		//submit_csd(fd, phyaddr, handle);
		submit_csd(fd, codeMem.getAddress(), codeMem.getHandle());
		wait_bo(fd, handle);
		double end = get_time();
		printf("[submit done: %.6lf sec]\n", end - start);

    {
        int res = munmap(usraddr, sizeof(do_nothing));
        assert(res == 0);
    }

    gem_close cl;
    cl.handle = handle;
    ioctl(fd, IOCTL_GEM_CLOSE, &cl);
}


#endif  // QPU_MODE

