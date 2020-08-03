#include "v3d/dump_instr.h"
#include <cstdio>
#include "summation.h"


std::vector<uint64_t> summation = {
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


namespace {
using Vec = std::vector<uint64_t>;
struct v3d_device_info devinfo;  // NOTE: uninitialized struct, field 'ver' must be set! For test OK

class Instr : public v3d_qpu_instr {
public:
	Instr(uint64_t code) {
		init(code);
	}

	void dump() const {
		char buffer[10*1024];
		instr_dump(buffer, const_cast<Instr *>(this));

		printf("%llu:\n%s", code(), buffer);
	}

	uint64_t code() const {
		init_ver();

    uint64_t repack = instr_pack(&devinfo, const_cast<Instr *>(this));
		return repack;
	}


	operator uint64_t() const {
		return code();
	}


private:
	void init_ver() const {
		if (devinfo.ver != 42) {               //        <-- only this needs to be set
			devinfo.ver = 42;
		}
	}

	void init(uint64_t code) {
		init_ver();

		// These do not get initialized in unpack
		sig_addr = 0;

		if (!instr_unpack(&devinfo, code, this)) {
			assert(false);
		}
	}
};


v3d_qpu_waddr r0 = V3D_QPU_WADDR_R0;
v3d_qpu_mux mux_r0 = V3D_QPU_MUX_R0; // TODO: get rid of prefix

Instr const nop(0x3c003186bb800000);  // This is actually 'nop nop'


Instr ldunifrf(uint8_t rf_address) {
	Instr instr(nop);

	instr.sig.ldunifrf = true; 
	instr.sig_addr = rf_address; 

	return instr;
}


Instr tidx(v3d_qpu_waddr reg) {
	Instr instr(nop);

	instr.sig_magic  = true; 
	instr.alu.add.op = V3D_QPU_A_TIDX;
	instr.alu.add.waddr = reg;
	instr.alu.add.a  = V3D_QPU_MUX_R1;
	instr.alu.add.b  = V3D_QPU_MUX_R0;

	return instr;
}


// TODO: where should reg2 go??
Instr shr(v3d_qpu_waddr reg1, v3d_qpu_waddr reg2, uint8_t val) {
	Instr instr(nop);

	instr.sig.small_imm = true; 
	instr.sig_magic     = true; 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_SHR;
	instr.alu.add.waddr = reg1;
	instr.alu.add.b     = V3D_QPU_MUX_B;

	return instr;
}


// 'and' is a keyword
Instr band(uint8_t rf_address, v3d_qpu_mux reg, uint8_t val) {
	Instr instr(nop);

	instr.sig.small_imm = true; 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_AND;
	instr.alu.add.waddr = rf_address;
	instr.alu.add.a     = reg;
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.magic_write = false;

	return instr;
}


Vec &operator<<(Vec &a, uint64_t val) {
	a.push_back(val);	
	return a;
}


Vec &operator<<(Vec &a, Vec const &b) {
	a.insert(a.end(), b.begin(), b.end());
	return a;
}


void show_instr(uint64_t code) {
	Instr instr(code);
	instr.dump();
}


}  // anon namespace


/**
 * This follows the kernel from the python `vodeocore6` project
 *
 * Source: https://github.com/Idein/py-videocore6/blob/3c407a2c0a3a0d9d56a5d0953caa7b0a4e92fa89/examples/summation.py#L11
 */
std::vector<uint64_t> summation_kernel(uint8_t num_qpus) {
	uint64_t op = 0x3de031807c83e0c4;  // shl  r0, rf3, 4      ; nop                              
	nop.dump();
	show_instr(op);

	// adresses of uniforms in the register file
	enum : uint8_t {
		reg_length = 0,
		reg_src,
		reg_dst,
		reg_qpu_num,
		reg_stride,
		reg_sum
	};

	std::vector<uint64_t> ret;
	
	ret << ldunifrf(reg_length)
	    << ldunifrf(reg_src)
	    << ldunifrf(reg_dst);

	uint8_t num_qpus_shift = 0;

	if (num_qpus == 1) {
		num_qpus_shift = 0;

		assert(false);  // TODO
		//ret << mov(reg_qpu_num, 0);
	} else if (num_qpus == 8) {
		num_qpus_shift = 3;

		ret << tidx(r0)
		    << shr(r0, r0, 2)
		    << band(reg_qpu_num, mux_r0, 0b1111);
	} else {
		assert(false);  // num_qpus must be 1 or 8A
	}

	return ret;
}
