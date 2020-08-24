#ifndef _QPULIB_V3D_INSTR_SNIPPETS_H
#define _QPULIB_V3D_INSTR_SNIPPETS_H
#include <vector>
#include "Instr.h"

namespace QPULib {
namespace v3d {
namespace instr {

using Instructions = std::vector<Instr>; 

Instructions enable_tmu_read(Instr const *last_slot);
Instructions sync_tmu();
Instructions end_program();

}  // instr
}  // v3d
}  // QPULib

#endif  // _QPULIB_V3D_INSTR_SNIPPETS_H
