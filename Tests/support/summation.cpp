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
