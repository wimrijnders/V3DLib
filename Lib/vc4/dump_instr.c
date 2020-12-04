#include "dump_instr.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DIAG_STRINGIFY(x) #x
#define WIGNORE(flag) _Pragma(DIAG_STRINGIFY(GCC diagnostic ignored #flag))

// NOTE: Following for gcc only
// GCC - should be ok for gcc v6.3.0 and later (currently using v8.3.0)i
//     - explicit v4.x def's dropped

// Disable specific warnings for the encompassed includes
#define DISABLE_COMPILE_TIME_WARNINGS \
  _Pragma("GCC diagnostic push") \
  WIGNORE(-Wconversion) \
  WIGNORE(-Wnarrowing) \
  WIGNORE(-Wsign-conversion) \

// Enable original warnings again
#define ENABLE_COMPILE_TIME_WARNINGS \
  _Pragma("GCC diagnostic pop")


DISABLE_COMPILE_TIME_WARNINGS
#define HAVE_ENDIAN_H
#include "gallium/drivers/vc4/vc4_qpu.h"  // vc4_qpu_disasm()
ENABLE_COMPILE_TIME_WARNINGS


/**
 * Output the vc4 mnemonics, redirecting to the output file.
 *
 * Mesa function `vc4_qpu_disasm()` outputs to stderr; this functions is a wrapper
 * to redirect `stderr` to the file we want.
 * 
 * Adapted from: https://www.unix.com/programming/268879-c-unix-how-redirect-stdout-file-c-code.html
 *
 * TODO Consider using memory streams: https://linux.die.net/man/3/open_memstream
 */
static int redirect(FILE *f, const uint64_t *instructions, int num_instructions) {
	// Start redirect
	int save_err = dup(fileno(stderr));
	int err = dup2(fileno(f), fileno(stderr));
	assert(err != -1);

	// Do the thing you want to do
	vc4_qpu_disasm(instructions, num_instructions);

	// End redirect	
	fflush(stderr); close(err);
	dup2(save_err, fileno(stderr));
	close(save_err);

	return 0;
}


void dump_instr(FILE * f,const uint64_t *instructions, int num_instructions) {
	redirect(f, instructions, num_instructions);
}
