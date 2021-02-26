#include "summation_kernel.h"
#include <cstdio>
#include "v3d/instr/Snippets.h"

namespace {

template<typename T>
using SharedArray =  V3DLib::SharedArray<T>;

/*
void check_returned_registers(SharedArray<uint32_t> &Y) {
  uint32_t cur_QPU;

  for (uint32_t offset = 0; offset < Y.size(); ++offset) {
    uint32_t this_QPU = offset / 16;

    if (this_QPU != cur_QPU) {
      printf("\n");
      cur_QPU = this_QPU;
      printf("%u: ", cur_QPU);
    } 

    bool used = Y[offset] != 0;
    if (used) {
      printf("%u, ", offset % 16);
    }
  }

  printf("\n");
}
*/


using Instructions = V3DLib::v3d::Instructions;

/**
 * Determine address offset for address registers.
 *
 * The offset is put in r0.
 * A register file location is also used as a temp storage location.
 *
 * @param reg_qpu_num index in the register file for location to put the qpu num in
 */
Instructions calc_offset(uint8_t num_qpus, uint8_t reg_qpu_num) {
  using namespace V3DLib::v3d::instr;
  Instructions ret;

  const char *text = 
    "Determine offset -> r0\n"
    "addr += 4 * (thread_num + 16 * qpu_num)";

  ret << set_qpu_num(num_qpus, reg_qpu_num).comment(text)
      << shl(r0, rf(reg_qpu_num), 4)
      << eidx(r1)
      << add(r0, r0, r1)
      << shl(r0, r0, 2);

  return ret;
}


/**
 * Calculates stride and start address per QPU
 *
 * @param reg_stride rf slot in which to store the stride
 */
Instructions calc_stride( uint8_t num_qpus, uint8_t reg_stride) {
  using namespace V3DLib::v3d::instr;
  Instructions ret;

  uint8_t num_qpus_shift = get_shift(num_qpus);

  ret << mov(rf(reg_stride), 1).header("stride = 4 * 16 * num_qpus")
      << shl(rf(reg_stride), rf(reg_stride), 6 + num_qpus_shift);

  return ret;
}

}  // anon namespace


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

using namespace V3DLib::v3d::instr;
using Instructions = V3DLib::v3d::Instructions;
using ByteCode = V3DLib::v3d::ByteCode;


Instructions adjust_length_for_unroll(uint8_t num_qpus, int unroll_shift, uint8_t reg_length) {
  Instructions ret;

  uint8_t num_qpus_shift = get_shift(num_qpus);

  // The QPU performs shifts and rotates modulo 32, so it actually supports
  // shift amounts [0, 31] only with small immediates.
  int num_shifts[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1};

  // length /= 16 * 8 * num_qpus * unroll
  ret << shr(rf(reg_length), rf(reg_length), num_shifts[7 + num_qpus_shift + unroll_shift]);

  return ret;
}


/**
 * @param code_offset  absolute offset of current instruction in the BO
 */
Instructions align_code(int code_offset, int target_offset) {
  Instructions ret;

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


Instructions emit_unroll(int unroll, Instructions const &block) {
  Instructions ret;

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
ByteCode summation_kernel(uint8_t num_qpus, int unroll_shift, int code_offset) {
  using namespace V3DLib::v3d::instr;

  Instructions ret;
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
  auto prefetch = mov(tmua, rf(reg_src)).add(rf(reg_src), rf(reg_src), rf(reg_stride));

  // alu's are working in parallel
  //   add: reg_sum += r0
  //   mul: load next slice in r0
  auto sum_and_load = add(rf(reg_sum), rf(reg_sum), r0).ldtmu(r0);


  //
  // Start of program emission
  //
  ret << nop().ldunifrf(rf(reg_length))
      << nop().ldunifrf(rf(reg_src))
      << nop().ldunifrf(rf(reg_dst))
      << calc_offset(num_qpus, reg_qpu_num)                     // Puts offset in r0
      << add(rf(reg_src), rf(reg_src), r0).add(rf(reg_dst), rf(reg_dst), r0)
      << calc_stride(num_qpus, reg_stride)
      << adjust_length_for_unroll(num_qpus, unroll_shift, reg_length)

      << enable_tmu_read(
           &bxor(rf(reg_sum), 1, 1).mov(r1, 1)                  // Passed opcode fills last delay slot
         )

      << align_code((int) (ret.size() + code_offset), 170);    // Why the magic number?
                                                               // TODO: See what happens if this is left out

  int loop_start = (int) ret.size();                           // Remember cur pos so that the loop can jump to it

  // Preload the slots in the TMU for faster accessing.
  // the eighth slot is filled in the next statements
  for (int i = 0; i < 7; ++i) {
    ret << prefetch;
  }

  ret << mov(tmua, rf(reg_src)).sub(rf(reg_length), rf(reg_length), r1).pushz()  // pushz sets flag for cond na0
      << add(rf(reg_src), rf(reg_src), rf(reg_stride)).ldtmu(r0);

  ret << emit_unroll(unroll, Instructions({prefetch, sum_and_load}));

  for (int i = 0; i < 5; ++i) {
    ret << sum_and_load;
  }

  ret << branch(loop_start, (int) ret.size()).na0() // Loop condition
      << sum_and_load                               // delay slot
      << sum_and_load                               // delay slot
      << add(rf(reg_sum), rf(reg_sum), r0)          // delay slot, last sum without load

      << mov(tmud, rf(reg_sum))                     // Write final result back to main mem
      << mov(tmua, rf(reg_dst))

      << sync_tmu()
      << end_program();

  ByteCode bytecode;
  for (int i = 0; i < (int) ret.size(); ++i ) {
    bytecode << ret[i].code(); 
  }

  return bytecode;
}

//
// Adapted from: https://github.com/Idein/py-videocore6/blob/master/examples/summation.py
//
// This uses a single shared array for code and data.
// It might be possible to use muliple arrays, but we're sticking to the original
// example here.
//
void run_summation_kernel(ByteCode &bytecode, uint8_t num_qpus, int unroll_shift) {
  using namespace V3DLib::v3d;

  //printf("bytecode size: %u\n", bytecode.size());

  REQUIRE((num_qpus == 1 || num_qpus == 8));

  uint32_t length = 32 * 1024 * 16;  // Highest number without overflows on 8 QPU's and CPU
                                     // The python version went to 32*1024*1024 and did some modulo magic.

  if (num_qpus == 1) {
    length = 32 * 1024 * 8;  // Highest number without overflows for 1 QPU
  }

  REQUIRE(length > 0);
  REQUIRE(length % (16 * 8 * num_qpus * (1 << unroll_shift)) == 0);

  //printf("==== summation example (%dK elements) ====\n", (length / 1024));

  // Code and data is combined in one buffer
  uint32_t code_area_size = (uint32_t) (8*bytecode.size());  // size in bytes
  //printf("code_area_size size: %u\n", code_area_size);
  uint32_t data_area_size = (length + 1024) * 4;
  //printf("data_area_size size: %u\n", data_area_size);

  BufferObject heap(code_area_size + data_area_size);
  //printf("heap phyaddr: %u, size: %u\n", heap.phy_address(), heap.size());

  heap.fill(0xdeadbeef);

  SharedArray<uint64_t> code((uint32_t) bytecode.size(), heap);
  code.copyFrom(bytecode);
  //printf("code phyaddr: %u, size: %u\n", code.getAddress(), 8*code.size());
  //dump_data(code); 

  SharedArray<uint32_t> X(length, heap);
  SharedArray<uint32_t> Y(16 * num_qpus, heap);
  //printf("X phyaddr: %u, size: %u\n", X.getAddress(), 4*X.size());
  //printf("Y phyaddr: %u, size: %u\n", Y.getAddress(), 4*Y.size());

  auto sumY = [&Y] () -> uint64_t {
    uint64_t ret = 0;

    for (uint32_t offset = 0; offset < Y.size(); ++offset) {
      ret += Y[offset];
    }

    return ret;
  };

  for (uint32_t offset = 0; offset < X.size(); ++offset) {
    X[offset] = offset;
  }
  //dump_data(X); 

  for (uint32_t offset = 0; offset < Y.size(); ++offset) {
    Y[offset] = 0;
  }
  //dump_data(Y); 
  REQUIRE(sumY() == 0);

  SharedArray<uint32_t> unif(3, heap);
  unif[0] = length;
  unif[1] = X.getAddress();
  unif[2] = Y.getAddress();
  //printf("unif phyaddr: %u, size: %u\n", unif.getAddress(), 4*unif.size());

  V3DLib::v3d::Driver drv;
  drv.add_bo(heap);
  REQUIRE(drv.execute(code, &unif, num_qpus));

  //dump_data(Y, true);
  //check_returned_registers(Y);
  //heap.detect_used_blocks();

/*
  // Check if code not overwritten
  for (uint32_t offset = 0; offset < summation.size(); ++offset) {
    INFO("Code offset: " << offset);
    REQUIRE(code[offset] == summation[offset]);
  }

  // Check if X not overwritten
  for (uint32_t offset = 0; offset < X.size(); ++offset) {
    INFO("X offset: " << offset);
    REQUIRE(X[offset] == offset);
  }

  heap.find_value(1736704u); // 4278190080u;
*/
  
  // Check if values supplied
  REQUIRE(sumY()  == 1llu*(length - 1)*length/2);
}

