#ifndef _V3DLIB_V3D_INSTR_SNIPPETS_H
#define _V3DLIB_V3D_INSTR_SNIPPETS_H
#include <vector>
#include "Instr.h"

namespace V3DLib {
namespace v3d {
namespace instr {

using Instructions = std::vector<Instr>; 

uint8_t get_shift(uint64_t num_qpus);
Instructions set_qpu_id(uint8_t reg_qpu_id);
Instructions set_qpu_num(uint8_t num_qpus, uint8_t reg_qpu_num);
Instructions get_num_qpus(Register const &reg, uint8_t num_qpus);
Instructions calc_offset( uint8_t num_qpus, uint8_t reg_qpu_num);
Instructions calc_stride( uint8_t num_qpus, uint8_t reg_stride);
Instructions enable_tmu_read(Instr const *last_slot = nullptr);
Instructions sync_tmu();
Instructions end_program();

}  // instr
}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_SNIPPETS_H
