#ifndef _V3DLIB_V3D_INSTR_LOCATION_H
#define _V3DLIB_V3D_INSTR_LOCATION_H
#include "broadcom/qpu/qpu_instr.h"

namespace V3DLib {
namespace v3d {
namespace instr {

class Location {
public:
  v3d_qpu_output_pack output_pack() const { return m_output_pack; }
  v3d_qpu_input_unpack input_unpack() const { return m_input_unpack; }
  bool is_rf() const { return m_is_rf; }

  virtual Location *clone() const = 0;
  virtual v3d_qpu_waddr to_waddr() const = 0;
  virtual v3d_qpu_mux to_mux() const = 0;
  virtual bool operator==(Location const &rhs) const = 0;

protected:
  v3d_qpu_output_pack m_output_pack   = V3D_QPU_PACK_NONE;
  v3d_qpu_input_unpack m_input_unpack = V3D_QPU_UNPACK_NONE;
  bool m_is_rf = false;
};

}  // instr
}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_LOCATION_H
