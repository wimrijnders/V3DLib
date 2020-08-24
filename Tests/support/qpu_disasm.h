#ifndef _TESTS_SUPPORT_QPU_DISASM_H_
#define _TESTS_SUPPORT_QPU_DISASM_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
        int ver;
        uint64_t inst;
        const char *expected;
} tests_struct; 

extern const tests_struct tests[]; 

extern const int tests_size;


#ifdef __cplusplus
}
#endif // __cplusplus


#endif  // _TESTS_SUPPORT_QPU_DISASM_H_
