// Source: https://github.com/Idein/py-videocore6/blob/master/videocore6/readwrite4.c

#if defined(__arm__) && defined(__aarch64__)
#error "__arm__ and __aarch64__ are both defined"
#elif !defined(__arm__) && !defined(__aarch64__)
#error "__arm__ and __aarch64__ are both not defined"
#endif


//#include <stdint.h>
#include "readwrite4.h"


uint32_t read4(void * const addr)
{
    uint32_t value;

    asm volatile (
#if defined(__arm__)
            "ldr %[value], [%[addr]]\n\t"
#elif defined(__aarch64__)
            "ldr %w[value], [%[addr]]\n\t"
#endif
            : [value] "=r" (value)
            : [addr] "r" (addr)
            : "memory"
    );

    return value;
}


void write4(void * const addr, const uint32_t value)
{
    asm volatile (
#if defined(__arm__)
            "str %[value], [%[addr]]\n\t"
#elif defined(__aarch64__)
            "str %w[value], [%[addr]]\n\t"
#endif
            :
            : [value] "r" (value),
              [addr] "r" (addr)
            : "memory"
    );
}

