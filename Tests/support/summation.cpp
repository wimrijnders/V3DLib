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

using namespace QPULib::v3d::instr;
using ByteCode = std::vector<uint64_t>; 


ByteCode end_program() {
	ByteCode ret;

	// Program tail
	ret << nop().thrsw(true)
	    << nop().thrsw(true)
	    << nop()
	    << nop()
	    << nop().thrsw(true)
	    << nop()
	    << nop()
	    << nop();

	return ret;
}


/**
 * Calculates stride and also start address per QPU
 *
 * Also sets the address offset for src and dsg registers
 * (Would prefer to have that outside of this routine)
 */
ByteCode calc_stride(
	uint8_t num_qpus,
	int     unroll_shift,
	uint8_t reg_qpu_num,
	uint8_t reg_src,
	uint8_t reg_dst,
	uint8_t reg_stride,
	uint8_t reg_length
) {
	ByteCode ret;

	uint8_t num_qpus_shift = 0;

	if (num_qpus == 1) {
		num_qpus_shift = 0;

		ret << mov(rf(reg_qpu_num), 0);
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
   ret << mov(rf(reg_stride), 1)
       << shl(reg_stride, reg_stride, 6 + num_qpus_shift);

	// The QPU performs shifts and rotates modulo 32, so it actually supports
	// shift amounts [0, 31] only with small immediates.
  int num_shifts[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		-16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1};

	// length /= 16 * 8 * num_qpus * unroll
	ret << shr(reg_length, reg_length, num_shifts[7 + num_qpus_shift + unroll_shift]);

	return ret;
}


/**
 * An instruction is passed in to make use of a waiting slot.
 */
ByteCode enable_tmu_read(Instr const &last_slot) {
	ByteCode ret;

	// This single thread switch and two instructions just before the loop are
	// really important for TMU read to achieve a better performance.
	// This also enables TMU read requests without the thread switch signal, and
	// the eight-depth TMU read request queue.
	ret << nop().thrsw(true)
	    << nop() 
			<< last_slot;

	return ret;
}


ByteCode sync_tmu() {
	ByteCode ret;

	// This synchronization is needed between the last TMU operation and the
	// program end with the thread switch just before the loop above.
	ret << barrierid(syncb).thrsw(true)
	    << nop()
	    << nop();

	return ret;
}


/**
 * @param code_offset  absolute offset of current instruction in the BO
 */
ByteCode align_code(int code_offset, int target_offset) {
	ByteCode ret;

	// This was actually the default of a parameter in the python version
	auto align_cond = [target_offset] (int pos) -> bool {
		return (pos % 512) == target_offset;
	};

	while (!align_cond(code_offset)) {
		ret << nop();
		code_offset++;
	}

	return ret;
}


ByteCode emit_unroll(int unroll, ByteCode block) {
	ByteCode ret;

	for (int j = 0; j < unroll - 1; ++j) {
		for (int i = 0; i < 8; ++i) {
			ret << block;
		}
	}

	return ret;
}

}  // anon namespace


/**
 * This follows the kernel from the python `videocore6` project
 *
 * Source: https://github.com/Idein/py-videocore6/blob/3c407a2c0a3a0d9d56a5d0953caa7b0a4e92fa89/examples/summation.py#L11
 */
std::vector<uint64_t> summation_kernel(uint8_t num_qpus, int unroll_shift, int code_offset) {
	using namespace QPULib::v3d::instr;

	std::vector<uint64_t> ret;
	int unroll = 1 << unroll_shift;

	// adresses of uniforms in the register file
	enum : uint8_t {
		reg_length = 0,
		reg_src,
		reg_dst,
		reg_qpu_num,
		reg_stride,
		reg_sum
	};

	//
	// Recurring operations
	//

	// alu's are working in parallel
	//   add: load TMU slot with address in reg_src
	//   mul: reg_src += reg_stride
	auto prefetch = mov(tmua, reg_src).add(reg_src, reg_src, reg_stride);

	// alu's are working in parallel
	//   add: reg_sum += r0
	//   mul: load next slice in r0
	auto sum_and_load =	add(reg_sum, reg_sum, r0).ldtmu(r0);


	//
	// Start of program emission
	//
	ret << ldunifrf(reg_length)
	    << ldunifrf(reg_src)
	    << ldunifrf(reg_dst)
	    << calc_stride(num_qpus, unroll_shift, reg_qpu_num, reg_src, reg_dst, reg_stride, reg_length)

	    << enable_tmu_read(
	       	bxor(reg_sum, 1, 1).mov(r1, 1)                       // Fills last delay slot
	       )

	    << align_code(ret.size() + code_offset, 170);            // Why the magic number?
	                                                             // TODO: See what happens if this is left out

	int loop_start = ret.size();                                 // Remember cur pos so that the loop can jump to it

	// Preload the slots in the TMU for faster accessing.
	// the eighth slot is filled in the next statements
	for (int i = 0; i < 7; ++i) {
		ret << prefetch;
	}

	ret << mov(tmua, reg_src).sub(reg_length, reg_length, r1).pushz()  // Apparently pushz sets flag for cond na0
			<< add(reg_src, reg_src, reg_stride).ldtmu(r0);

	ret << emit_unroll(unroll, {
			prefetch,
			sum_and_load
	});

	for (int i = 0; i < 5; ++i) {
		ret << sum_and_load;
	}

	ret << branch(loop_start, ret.size()).cond_na0()  // Loop condition
	    << sum_and_load                               // delay slot
	    << sum_and_load                               // delay slot
	    << add(reg_sum, reg_sum, r0)                  // delay slot, last sum without load

	    << mov(tmud, reg_sum)                         // Write final result back to main mem
      << mov(tmua, reg_dst)

	    << sync_tmu()
	    << end_program();

	return ret;
}
