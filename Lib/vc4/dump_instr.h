#ifndef _LIB_VC4_DUMP_INSTR_H
#define _LIB_VC4_DUMP_INSTR_H
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void dump_instr(FILE *f, const uint64_t *instructions, int num_instructions);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // _LIB_VC4_DUMP_INSTR_H
