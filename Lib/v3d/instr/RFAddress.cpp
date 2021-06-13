#include "RFAddress.h"
#include "../../Support/debug.h"

namespace V3DLib {
namespace v3d {
namespace instr {


v3d_qpu_mux RFAddress::to_mux() const {
  return V3D_QPU_MUX_A;
}


RFAddress RFAddress::l() const {
  RFAddress ret(m_val);
  ret.m_input_unpack = V3D_QPU_UNPACK_L;
  ret.m_output_pack = V3D_QPU_PACK_L;
  assert(ret.is_rf());

  return ret;
}


RFAddress RFAddress::h() const {
  RFAddress ret(m_val);
  ret.m_input_unpack = V3D_QPU_UNPACK_H;
  ret.m_output_pack = V3D_QPU_PACK_H;

  return ret;
}


RFAddress RFAddress::abs() const {
  RFAddress ret(m_val);
  ret.m_input_unpack = V3D_QPU_UNPACK_ABS;
  ret.m_output_pack = V3D_QPU_PACK_NONE;  // PACK_ABS does not exist (logical)

  return ret;
}


bool RFAddress::operator==(Location const &rhs) const {
  RFAddress const *rhs_rf = dynamic_cast<RFAddress const *>(&rhs);
  if (rhs_rf == nullptr) return false;

  assert(m_is_rf);
  assert(m_is_rf == rhs_rf->m_is_rf);

  if (m_output_pack != rhs_rf->m_output_pack || m_input_unpack != rhs_rf->m_input_unpack) {
    warning("RFAddress::==(): packing differs");
  }

  return m_val == rhs_rf->m_val;
}

}  // instr
}  // v3d
}  // V3DLib
