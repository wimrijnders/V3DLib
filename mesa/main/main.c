/**
 * Goal of this file is to initiate the proper linking for the required functions
 *
 */
#include <stdio.h>
#include "gallium/drivers/v3d/v3d_context.h"
#include "loader/loader.h"

#include <signal.h>  // raise(SIGTRAP);
#define breakpoint raise(SIGTRAP);

/*
// TEST1

#include "broadcom/compiler/v3d_compiler.h"

void debug_output(const char *msg, void *debug_output_data) {
}

void compile1() {
	struct v3d_compiler compiler;
	struct v3d_key key;
	struct v3d_prog_data *prog_data = 0;
	nir_shader s;
	void *debug_output_data = 0;
	int program_id = 0;
	int variant_id = 0;
	uint32_t final_assembly_size;

	uint64_t *ret = v3d_compile(
		&compiler,
		&key,
		&prog_data,
		&s,
    debug_output,
		debug_output_data,
		program_id, variant_id,
		&final_assembly_size);

	printf("ret: %llu\n", *ret);
}


// TEST2

void compile2() {
	struct v3d_context v3d;
	v3d_update_compiled_cs(&v3d);
*/


// TEST3

// NOTE:
// pipe_loader_create_screen() in gallium/auxiliary/pipe-loader/pipe_loader.c

#include "gallium/include/frontend/drm_driver.h"
#include "gallium/winsys/v3d/drm/v3d_drm_public.h"
#include "util/driconf.h"
#include "gallium/auxiliary/pipe-loader/pipe_loader.h"

// Adjusted from gallium/auxiliary/target-helpers/drm_helper.h
struct pipe_screen *
pipe_v3d_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = v3d_drm_screen_create(fd, config);
   //return screen ? debug_screen_wrap(screen) : NULL;
   return screen;
}

const char *v3d_driconf_xml =
      //#include "v3d/v3d_driinfo.h"  // WR: Is this a typo in the mesa code??
      #include "gallium/drivers/v3d/driinfo_v3d.h"
      ;

// from gallium/auxiliary/pipe-loader/pipe_loader_drm.c
const struct drm_driver_descriptor driver_descriptor = {
        .driver_name = "v3d",
        .create_screen = pipe_v3d_create_screen,
        .driconf_xml = &v3d_driconf_xml,
};


/**
 prob TODO: call one/bothof following to clean up

gallium/auxiliary/pipe-loader/pipe_loader.h:pipe_loader_release(struct pipe_loader_device **devs, int ndev);
gallium/auxiliary/pipe-loader/pipe_loader_drm.c:   pipe_loader_base_release(dev);
 */

int main(int argc, char *argv[]) {
  const char *CARD_0  =  "/dev/dri/card0";
  const char *CARD_1  =  "/dev/dri/card1";

	int fd;  // TODO initialize

	struct pipe_screen_config *config = 0;
	struct renderonly *ro = 0;  // can be passed as null

	struct pipe_screen *pscreen = 0;
	void *priv = 0;
	unsigned flags = 0;
	struct pipe_context *pcontext;

	// Called in v3d_drm_screen_create()
	//pscreen = v3d_screen_create(fd, config, ro);

	fd	= loader_open_device(CARD_0);
	if (fd == -1) {
		exit(-1);
	}

	struct pipe_loader_device *dev = 0;
	if (!pipe_loader_drm_probe_fd(&dev, fd)) {
		exit(-1);  // FAIL
	}

	breakpoint
	pscreen = pipe_loader_create_screen(dev);
	pcontext = v3d_context_create(pscreen, priv, flags);

	return 0;
}
