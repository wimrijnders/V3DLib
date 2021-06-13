#ifndef _V3DLIB_V3D_INSTR_SMALLIMM_H
#define _V3DLIB_V3D_INSTR_SMALLIMM_H
#include <stdint.h>
#include <string>
#include "broadcom/qpu/qpu_instr.h"
#include "dump_instr.h"

namespace V3DLib {
namespace v3d {
namespace instr {


class SmallImm {
public:
  SmallImm(int val, bool is_val = true);

  uint8_t to_raddr() const;
  v3d_qpu_input_unpack input_unpack() const { return m_input_unpack; }
  int val() const;  // for assertions
  bool operator==(SmallImm const &rhs) const;

  SmallImm l() const;
  SmallImm ff() const;

  static bool int_to_opcode_value(int value, int &rep_value);
  static bool float_to_opcode_value(float value, int &rep_value);
  static bool is_legal_encoded_value(int value);
  static std::string print_encoded_value(int value);

private:
  int m_val = 0;
  bool m_val_is_set = false;
  uint8_t m_index = 0xff;  // init to illegal value
  v3d_qpu_input_unpack m_input_unpack = V3D_QPU_UNPACK_NONE;

  void pack();
};

}  // instr
}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_SMALLIMM_H
