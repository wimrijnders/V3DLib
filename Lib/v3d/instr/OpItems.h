#ifndef _V3DLIB_V3D_INSTR_OPITEMS_H
#define _V3DLIB_V3D_INSTR_OPITEMS_H
#include "Target/instr/Instr.h"
#include "dump_instr.h"

namespace V3DLib {
namespace v3d {
namespace instr {

class OpItems {
public:
  static bool get_add_op(ALUInstruction const &add_alu, v3d_qpu_add_op &dst);
  static bool get_mul_op(ALUInstruction const &add_alu, v3d_qpu_mul_op &dst);
  static bool valid_combine_pair(V3DLib::Instr const &instr, V3DLib::Instr const &next_instr, bool &do_converse);

private:
  static bool uses_add_alu(V3DLib::Instr const &instr);
  static bool uses_mul_alu(V3DLib::Instr const &instr);
};


}  // namespace instr
}  // namespace v3d
}  // namespace V3DLib

#endif  // _V3DLIB_V3D_INSTR_OPITEMS_H
