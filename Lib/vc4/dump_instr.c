#include "dump_instr.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "disasm.h"


/**
 * Output the vc4 mnemonics, redirecting to the output file.
 *
 * Mesa function `vc4_qpu_disasm()` outputs to stderr; this functions is a wrapper
 * to redirect `stderr` to the file we want.
 * 
 * Adapted from: https://www.unix.com/programming/268879-c-unix-how-redirect-stdout-file-c-code.html
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * Tried out `open_memstream()`  - https://linux.die.net/man/3/open_memstream
 *   Doesn't work:
 *
 *    FILE *out;
 *    char *ptr;
 *    size_t size;
 *
 *    out = open_memstream(&ptr, &size);
 *    assert(out != NULL);
 *  
 *    // Start redirect
 *    int save_err = dup(fileno(stderr));
 *    int err = dup2(fileno(out), fileno(stderr));
 *    assert(err != -1);  // <-- fails
 */
static void redirect(FILE *f, const uint64_t *instructions, int num_instructions) {
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
}


/**
 * Capture stderr output of enclosed function `vc4_qpu_disasm()` and redirect
 * to passed file.
 */
void dump_instr(FILE * f, const uint64_t *instructions, int num_instructions) {
  redirect(f, instructions, num_instructions);
}
