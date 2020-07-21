#ifndef _VC6_READWRITE4_H
#define _VC6_READWRITE4_H
#include <stdint.h>

uint32_t read4(void * const addr);
void write4(void * const addr, const uint32_t value);


#endif  // _VC6_READWRITE4_H
