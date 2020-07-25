/**
 * Goal of this file is to initiate the proper linking for the required functions
 *
 */
#include <stdio.h>
#include "gallium/drivers/v3d/v3d_context.h"

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

int main(int argc, char *argv[]) {
	int fd = 0;
	struct pipe_screen_config *config = 0;
	struct renderonly *ro = 0;

	struct pipe_screen *pscreen;
	void *priv = 0;
	unsigned flags = 0;
	struct pipe_context *pcontext;

	pscreen = v3d_screen_create(fd, config, ro);
	pcontext = v3d_context_create(pscreen, priv, flags);

	return 0;
}
