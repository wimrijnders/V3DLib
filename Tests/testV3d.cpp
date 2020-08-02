#include "catch.hpp"
#ifdef QPU_MODE
#include <sys/time.h>
#include <cstring>
#include "v3d/SharedArray.h"
#include "v3d/v3d.h"
#include "v3d/dump_instr.h"
#include "debug.h"
#include "Support/Platform.h"

#define ARRAY_LENGTH(arr, type) (sizeof(arr)/sizeof(type))

using Data = QPULib::v3d::SharedArray<uint32_t>;


namespace {

const uint32_t DEFAULT_CODE_AREA_SIZE = 1024 * 1024;
const uint32_t DEFAULT_DATA_AREA_SIZE = 32 * 1024 * 1024;


// Note that do_nothing is an array of 64-bit values!
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


uint64_t summation[] = {
	0x3d803186bb800000,  // nop                  ; nop               ; ldunifrf.rf0 
	0x3d807186bb800000,  // nop                  ; nop               ; ldunifrf.rf1 
	0x3d80b186bb800000,  // nop                  ; nop               ; ldunifrf.rf2 
	0x3c003180bb801000,  // tidx  r0             ; nop                              
	0x3de031807d838002,  // shr  r0, r0, 2       ; nop                              
	0x3de02183b583800f,  // and  rf3, r0, 15     ; nop                              
	0x3de031807c83e0c4,  // shl  r0, rf3, 4      ; nop                              
	0x3c003181bb802000,  // eidx  r1             ; nop                              
	0x3c00318038808000,  // add  r0, r0, r1      ; nop                              
	0x3de031807c838002,  // shl  r0, r0, 2       ; nop                              
	0x04000081381c6042,  // add  rf1, rf1, r0    ; add  rf2, rf2, r0                
	0x3de02184b683f001,  // or  rf4, 1, 1        ; nop                              
	0x3de021847c83e109,  // shl  rf4, rf4, 9     ; nop                              
	0x3de021807d83e00f,  // shr  rf0, rf0, 15    ; nop                              
	0x3c203186bb800000,  // nop                  ; nop               ; thrsw        
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3de02045b7fff001,  // xor  rf5, 1, 1       ; mov  r1, 1                       
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x0804500cb63f6040,  // or  tmua, rf1, rf1   ; sub.pushz  rf0, rf0, r1          
	0x3c9021813883e044,  // add  rf1, rf1, rf4   ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x0400104cb6fb6044,  // or  tmua, rf1, rf1   ; add  rf1, rf1, rf4               
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x02ffeff3ff009000,  // b.na0  -4112                                            - Repack FAILED: 0x02ffeff3ff001000: "b.na0  -4112"
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x3c90218538806140,  // add  rf5, rf5, r0    ; nop               ; ldtmu.r0     
	0x3c00218538806140,  // add  rf5, rf5, r0    ; nop                              
	0x3c00318bb6836140,  // or  tmud, rf5, rf5   ; nop                              
	0x3c00318cb6836080,  // or  tmua, rf2, rf2   ; nop                              
	0x3c203192bb814000,  // barrierid  syncb     ; nop               ; thrsw        
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c203186bb800000,  // nop                  ; nop               ; thrsw        
	0x3c203186bb800000,  // nop                  ; nop               ; thrsw        
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c203186bb800000,  // nop                  ; nop               ; thrsw        
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
	0x3c003186bb800000,  // nop                  ; nop                              
};

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + t.tv_usec * 1e-6;
}


template<typename T>
void dump_data(T const &arr) {
	const int DISP_LENGTH = 4;

	int first_size = (int) arr.size();
	if (first_size > DISP_LENGTH) {
		first_size = DISP_LENGTH;
	}
		
	for (int offset = 0; offset < first_size; ++offset) {
		printf("%8d: %8d\n", offset, arr[offset]);
	}

	if (first_size == arr.size()) {
		return;
	}

	printf("      ...\n");

	for (int offset = arr.size() - DISP_LENGTH; offset < (int) arr.size(); ++offset) {
		printf("%8d: %8d\n", offset, arr[offset]);
	}

	printf("\n");
}

}  // anon namespace


/**
 * Adjusted from: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
 */
TEST_CASE("Check v3d code is working properly", "[v3d]") {
	SECTION("Direct v3d calls should work properly") {
		if (Platform::instance().has_vc4) {
			// Skip test if not on Pi4
			printf("Skipping v3d tests\n");
			return;
		}

		REQUIRE(v3d_open());

    uint32_t handle = 0;
    uint32_t phyaddr = 0;
		void *usraddr = nullptr;
		REQUIRE(v3d_alloc(sizeof(do_nothing), handle, phyaddr, &usraddr));
		REQUIRE(handle != 0);
		REQUIRE(phyaddr != 0);
		REQUIRE(usraddr != nullptr);

		uint32_t array_length = ARRAY_LENGTH(do_nothing, uint64_t);
		assert(array_length == 8);

    memcpy(usraddr, do_nothing, sizeof(do_nothing));

		//
		// NOTE: During testing, execution time shot up from 0.1 sec to 10 sec.
		//       This probably due to the gpu hanging because of previously job faulty (too short)
		//       The 10 sec is likely the timeout.
		//
		// TODO: Find a way to reset the v3d
		//
		double start = get_time();
		REQUIRE(v3d_submit_csd(phyaddr, handle));
		double end = get_time();
		printf("[submit done: %.6lf sec]\n", end - start);

		REQUIRE(v3d_unmap(sizeof(do_nothing), handle, usraddr));
	}


	SECTION("Direct v3d calls should work with SharedArray") {
		if (Platform::instance().has_vc4) {
			// Skip test if not on Pi4
			return;
		}

		REQUIRE(v3d_open());

		uint32_t array_length = ARRAY_LENGTH(do_nothing, uint64_t);
		assert(array_length == 8);

		QPULib::v3d::SharedArray<uint64_t> codeMem(array_length);

		for (int offset = 0; offset < array_length; ++offset) {
			codeMem[offset] = do_nothing[offset];
		}

		//
		// See note previous test
		//
		double start = get_time();
		REQUIRE(v3d_submit_csd(codeMem));
		double end = get_time();
		printf("[submit done: %.6lf sec]\n", end - start);
	}


	SECTION("v3d SharedArray should work as expected") {
		if (Platform::instance().has_vc4) {
			// Skip test if not on Pi4
			return;
		}

		const int SIZE = 16;

		QPULib::v3d::SharedArray<uint32_t> arr(SIZE);
		REQUIRE(arr.size() == SIZE);

		for(int offset = 0; offset < SIZE; ++offset) {
			arr[offset] = 0;
		}

		for(int offset = 0; offset < SIZE; ++offset) {
			REQUIRE(arr[offset] == 0);
		}


		for(int offset = 0; offset < SIZE; ++offset) {
			arr[offset] = 127;
		}

		for(int offset = 0; offset < SIZE; ++offset) {
			REQUIRE(arr[offset] == 127);
		}

		arr.dealloc();
		REQUIRE(arr.size() == 0);
	}


	//
	// Adapted from: https://github.com/Idein/py-videocore6/blob/master/examples/summation.py
	//
	// This uses a shared array for code and data.
	// This might not be necessary due to usage of BO's, but we're sticking to the original
	// example here.
	//
	SECTION("Summation example should work") {
		uint32_t length = 32 * 1024 * 1024;
		int num_qpus = 8;
		int unroll_shift = 5;

    REQUIRE(length > 0);
    REQUIRE(length % (16 * 8 * num_qpus * (1 << unroll_shift)) == 0);

    printf("==== summation example (%lld Mi elements) ====\n", (length / 1024 / 1024));

    //with Driver(data_area_size=(length + 1024) * 4) as drv:
    uint32_t code_area_size = DEFAULT_CODE_AREA_SIZE;
    uint32_t data_area_size = (length + 1024) * 4;

		printf("Preparing for buffers...\n");
		// Code and data is combined in one buffer
		Data heap(code_area_size + data_area_size);
		auto code = heap.alloc_view<uint64_t>(code_area_size);
		code.copyFrom(summation, ARRAY_LENGTH(summation, uint64_t));

		auto X = heap.alloc_view<uint32_t>(length);
		auto Y = heap.alloc_view<uint32_t>(16 * num_qpus);

		for (uint32_t offset = 0; offset < X.size(); ++offset) {
			X[offset] = offset;
		}

		dump_data(X); 

		for (uint32_t offset = 0; offset < Y.size(); ++offset) {
			Y[offset] = 0;
		}

		dump_data(Y); 

		auto sumY = [&Y] () -> uint32_t {
			uint32_t ret = 0;

			for (uint32_t offset = 0; offset < Y.size(); ++offset) {
				ret += Y[offset];
			}

			return ret;
		};

		REQUIRE(sumY() == 0);

		Data unif(3);
		unif[0] = length;
		unif[1] = X.getAddress();  // TODO: is this correct?
		unif[2] = Y.getAddress();  // TODO: is this correct?


		printf("Executing on QPU...\n");
		double start = get_time();

		// drv.execute(code, unif.addresses()[0], thread=num_qpus)
		//REQUIRE(v3d_submit_csd(combinedMem));
		REQUIRE(false);  // TODO

		dump_data(Y); 
    REQUIRE(sumY() % (1ull << 32) == (length - 1) * length); // 2 % 2**32
		double end = get_time();
		printf("Summation done: %.6lf sec, %.6lf MB/s\n", (end - start), (length * 4 / (end - start) * 1e-6));
	}
}


TEST_CASE("Check v3d assembly/disassembly", "[v3d][asm]") {
	SECTION("Correct output of dump program") {
		struct v3d_device_info devinfo;  // NOTE: uninitialized struct! For test OK
		devinfo.ver = 42;               //        <-- only this needs to be set

		const char *expected = "\n{\n\
  type: INSTR_TYPE_ALU,\n\
  sig: {ldunifrf },\n\
  sig_addr: 0,\n\
  sig_magic: false,\n\
  raddr_a: 0 ,\n\
  raddr_b: 0,\n\
  flags: {ac: COND_NONE, mc: COND_NONE, apf: PF_NONE, mpf: PF_NONE, auf: UF_NONE, muf: UF_NONE},\n\
  alu: {\n\
    add: {\n\
      op: A_NOP,\n\
      a: MUX_R0,\n\
      b: MUX_R0,\n\
      waddr: 6,\n\
      magic_write: true,\n\
      output_pack: PACK_NONE,\n\
      a_unpack: UNPACK_NONE, \n\
      b_unpack: UNPACK_NONE\n\
    },\n\
    mul: {\n\
      op: M_NOP,\n\
      a: MUX_R0,\n\
      b: MUX_R4,\n\
      waddr: 6,\n\
      magic_write: true,\n\
      output_pack: PACK_NONE,\n\
      a_unpack: UNPACK_NONE, \n\
      b_unpack: UNPACK_NONE\n\
    }\n\
  }\n\
}\n";

		uint64_t nop = 0x3d803186bb800000;  // nop                  ; nop               ; ldunifrf.rf0 

		struct v3d_qpu_instr instr;
		REQUIRE(!v3d_qpu_instr_unpack(&devinfo, nop, &instr));

		char buffer[10*1024];
		instr_dump(buffer, &instr);

		INFO("Expected:\n" << expected);
		INFO("Output:\n" << buffer);
		REQUIRE(!strcmp(expected, buffer));
	}
}

#undef ARRAY_LENGTH

#endif  // QPU_MODE

