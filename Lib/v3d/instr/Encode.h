#ifndef _V3DLIB_V3D_INSTR_ENCODE_H
#define _V3DLIB_V3D_INSTR_ENCODE_H
#include <memory>
#include "Target/instr/Instr.h"
#include "Location.h"
#include "RFAddress.h"

namespace V3DLib {
namespace v3d {
namespace instr {

uint8_t to_waddr(Reg const &reg);
//void check_unhandled_registers(Reg reg, bool do_src_regs);
std::unique_ptr<Location> encodeSrcReg(Reg reg);
std::unique_ptr<Location> encodeDestReg(V3DLib::Instr const &src_instr);

}  // instr
}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_ENCODE_H
