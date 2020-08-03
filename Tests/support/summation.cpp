#include "v3d/Instr.h"
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

Vec &operator<<(Vec &a, uint64_t val) {
	a.push_back(val);	
	return a;
}


Vec &operator<<(Vec &a, Vec const &b) {
	a.insert(a.end(), b.begin(), b.end());
	return a;
}




}  // anon namespace


/**
 * This follows the kernel from the python `vodeocore6` project
 *
 * Source: https://github.com/Idein/py-videocore6/blob/3c407a2c0a3a0d9d56a5d0953caa7b0a4e92fa89/examples/summation.py#L11
 */
std::vector<uint64_t> summation_kernel(uint8_t num_qpus, int unroll_shift) {
	using namespace QPULib::v3d::instr;

	uint64_t op = 0x3d803186bb800000;  // nop                  ; nop               ; ldunifrf.rf0 
	nop().dump();
	Instr::show(op);

	Instr::show(0x3d903186bb800000);

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

	ldunifrf(reg_length).dump();
	
	ret << ldunifrf(reg_length)
	    << ldunifrf(reg_src)
	    << ldunifrf(reg_dst);

	uint8_t num_qpus_shift = 0;

	if (num_qpus == 1) {
		num_qpus_shift = 0;

		ret << mov(reg_qpu_num, 0);
	} else if (num_qpus == 8) {
		num_qpus_shift = 3;

		ret << tidx(r0)
		    << shr(r0, r0, 2)
		    << band(reg_qpu_num, r0, 0b1111);
	} else {
		assert(false);  // num_qpus must be 1 or 8
	}

	// addr += 4 * (thread_num + 16 * qpu_num)
	ret << shl(r0, reg_qpu_num, 4)
	    << eidx(r1)
	    << add(r0, r0, r1)
	    << shl(r0, r0, 2)
	    << add(reg_src, reg_src, r0).add(reg_dst, reg_dst, r0);

   // stride = 4 * 16 * num_qpus
   ret << mov(reg_stride, 1)
       << shl(reg_stride, reg_stride, 6 + num_qpus_shift);

	// The QPU performs shifts and rotates modulo 32, so it actually supports
	// shift amounts [0, 31] only with small immediates.
  int num_shifts[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		-16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1};

	// length /= 16 * 8 * num_qpus * unroll
  ret << shr(reg_length, reg_length, num_shifts[7 + num_qpus_shift + unroll_shift]);

    bxor(reg_sum, 1, 1).mov(r1, 1).dump();

	// This single thread switch and two instructions just before the loop are
	// really important for TMU read to achieve a better performance.
	// This also enables TMU read requests without the thread switch signal, and
	// the eight-depth TMU read request queue.
	ret << nop().thrsw(true)
	    << nop() 
	    << bxor(reg_sum, 1, 1).mov(r1, 1);

	return ret;
}
