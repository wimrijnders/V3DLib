/**
 * Adjusted from: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
 */
#include "v3d.h"
#include <cassert>
#include <sys/ioctl.h>
#include <cstddef>    // NULL
#include <cstring>    // errno, strerror()
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>   // close()
#include "Support/basics.h"
#include "Support/debug.h"

namespace {

int fd = 0;

typedef struct {
    uint32_t size   = 0;
    uint32_t flags  = 0;
    uint32_t handle = 0;
    uint32_t offset = 0;
} drm_v3d_create_bo;

typedef struct {
    uint32_t handle = 0;
    uint32_t flags  = 0;
    uint64_t offset = 0;
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


void log_error(int ret, char const *prefix = "") {
  if (ret == 0) return;

  char buf[256];
  sprintf(buf, "%sioctl: %s\n", prefix, strerror(errno));
  error(buf);
}


/**
 * Allocate and map a buffer object
 *
 * This function is also used to check availability of the GPU device
 * via a given device driver (called 'card' in this code).
 * Parameter `show_perror` is used to suppress any errors if this check fails.
 *
 * @param show_perror  if true, suppress any errors when creating a buffer object
 */
bool alloc_intern(
  int fd,
  uint32_t size,
  uint32_t &handle,
  uint32_t &phyaddr,
  void **usraddr,
  bool show_perror = true) {
  assert(fd != 0);

  drm_v3d_create_bo create_bo;
  create_bo.size = size;
  create_bo.flags = 0;
  {
    int result = ioctl(fd, IOCTL_V3D_CREATE_BO, &create_bo);
    if (show_perror) {
      log_error(result, "alloc_intern() create bo ");  // `show_perror` intentionally only used here
    }
    if (result != 0) return false;
  }
  handle  = create_bo.handle;
  phyaddr = create_bo.offset;

  drm_v3d_mmap_bo mmap_bo;
  mmap_bo.handle = create_bo.handle;
  mmap_bo.flags = 0;
  {
    int result = ioctl(fd, IOCTL_V3D_MMAP_BO, &mmap_bo);
    log_error(result, "alloc_intern() mmap bo ");
    if (result != 0) return false;
  }

  {
    void *result = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (__off_t) mmap_bo.offset);
    if (result == MAP_FAILED) {
      log_error(1, "alloc_intern() mmap ");
      return false;
    }

    *usraddr = result;
  }

  return true;
}


void fd_close(int fd) {
  if (fd > 0 ) {
#ifdef DEBUG
    assert(close(fd) >= 0);
#else
    close(fd);
#endif
  }
}


int open_card(char const *card) {
  int fd = open(card , O_RDWR);

  if (fd == 0) {
    V3DLib::fatal("FATAL: Can't open card device (sudo?)");
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

    if (alloc_intern(fd, ALLOC_SIZE, handle, phyaddr, &usraddr, false)) {
      // worked! clean up
      v3d_unmap(ALLOC_SIZE, handle, usraddr);
      //printf("open_card(): alloc test succeeded for card %s\n", card);
    } else {
      // fail
      fd_close(fd);
      fd = 0;
      //printf("open_card(): alloc test FAILED for card %s\n", card);
    }
  }

  return fd;
}


bool v3d_wait_bo(uint32_t handle, uint64_t timeout_ns) {
  assert(handle != 0);
  assert(timeout_ns > 0);

  st_v3d_wait_bo st = {
    handle,
    0,
    timeout_ns,
  };

  int ret = ioctl(fd, IOCTL_V3D_WAIT_BO, &st);
  log_error(ret, "v3d_wait_bo() ");
  return (ret == 0);
}

}  // anon namespace


/**
 *
 */
int v3d_submit_csd(st_v3d_submit_csd &st) {
  int ret = ioctl(fd, IOCTL_V3D_SUBMIT_CSD, &st);
  log_error(ret, "v3d_submit_csd() ");
  assert(ret == 0);
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
    return true;  // Already open, all is well
  }

  // It appears to be a random crap shoot which device card0 and card1 address
  // So we try both, test them and pick the one that works (if any)
  int fd0 = open_card("/dev/dri/card0");
  int fd1 = open_card("/dev/dri/card1");

  if (fd0 == 0) {
    assertq(fd1 != 0, "Could not open v3d device, did you forget 'sudo'?");
    fd = fd1;
  } else if (fd1 == 0) {
    assertq(fd0 != 0, "Could not open v3d device, did you forget 'sudo'?");
    fd = fd0;
  } else {
    V3DLib::fatal("FATAL: could not open v3d device");
  }

  return (fd > 0);
}


/**
 * @return true if close executed, false if already closed
 */
bool v3d_close() {
  if (fd == 0) { return false; }

  fd_close(fd);
  fd = 0;

  return true;
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


/**
 * @return true if all waits succeeded, false otherwise
 */
bool v3d_wait_bo(std::vector<uint32_t> const &bo_handles, uint64_t timeout_ns) {
  assert(bo_handles.size() > 0);
  assert(timeout_ns > 0);

  int ret = true;

  for (auto handle : bo_handles) {
    if (!v3d_wait_bo(handle, timeout_ns)) { 
      ret = false;
    }
  }
  //printf("Done calling v3d_wait_bo()\n");

  return ret;
}
