#include "Register.h"
#include "Support/basics.h"  // Exception
#include "RFAddress.h"


namespace V3DLib {
namespace v3d {
namespace instr {

Register::Register(const char *name, v3d_qpu_waddr waddr_val) :
  m_name(name),
  m_waddr_val(waddr_val),
  m_mux_val(V3D_QPU_MUX_R0),  // Dummy value, set to first element in item
  m_mux_is_set(false)
{}


Register::Register(const char *name, v3d_qpu_waddr waddr_val, v3d_qpu_mux mux_val, bool is_dest_acc) :
  m_name(name),
  m_waddr_val(waddr_val),
  m_mux_val(mux_val),
  m_mux_is_set(true),
  m_is_dest_acc(is_dest_acc)
{}


v3d_qpu_mux Register::to_mux() const {
  if (!m_mux_is_set) {
    std::string buf;
    buf += "Can't use mux value for register '" + m_name + "', it is not defined";
    throw Exception(buf);
  }

  return m_mux_val;
}


Register Register::l() const {
  Register ret(*this);
  ret.m_input_unpack = V3D_QPU_UNPACK_L;
  ret.m_output_pack  = V3D_QPU_PACK_L;
  return ret;
}


Register Register::ll() const {
  Register ret(*this);
  ret.m_input_unpack = V3D_QPU_UNPACK_REPLICATE_L_16;
  return ret;
}


Register Register::hh() const {
  Register ret(*this);
  ret.m_input_unpack = V3D_QPU_UNPACK_REPLICATE_H_16;
  return ret;
}


Register Register::h() const {
  Register ret(*this);
  ret.m_input_unpack = V3D_QPU_UNPACK_H;
  ret.m_output_pack  = V3D_QPU_PACK_H;
  return ret;
}


Register Register::abs() const {
  Register ret(*this);
  ret.m_input_unpack = V3D_QPU_UNPACK_ABS;
  return ret;
}


Register Register::swp() const {
  Register ret(*this);
  ret.m_input_unpack = V3D_QPU_UNPACK_SWAP_16;
  return ret;
}


v3d_qpu_mux BranchDest::to_mux() const {
  std::string buf;
  buf += "Can't use mux value for Branch dest '" + m_name + "', it is not defined";
  throw Exception(buf);

  return (v3d_qpu_mux) 0;
}


bool BranchDest::operator==(Location const &rhs) const {
  BranchDest const *rhs_dst = dynamic_cast<BranchDest const *>(&rhs);
  if (rhs_dst == nullptr) return false;

  return (m_dest == rhs_dst->m_dest);
}


bool Register::operator==(Location const &rhs) const {
  RFAddress const *rhs_rf = dynamic_cast<RFAddress const *>(&rhs);
  if (rhs_rf != nullptr) {
    if (m_mux_is_set) {
      return (m_mux_val == V3D_QPU_MUX_A || m_mux_val == V3D_QPU_MUX_B) && m_waddr_val == rhs_rf->to_waddr();
    } else {
      return m_waddr_val == rhs_rf->to_waddr();
    }
  }


  Register const *rhs_reg = dynamic_cast<Register const *>(&rhs);
  if (rhs_reg == nullptr) return false;

  assert(!m_is_rf);
  assert(m_is_rf == rhs_reg->m_is_rf);

  if (m_output_pack != rhs_reg->m_output_pack || m_input_unpack != rhs_reg->m_input_unpack) {
    warning("Register::==(): packing differs");
  }

  if (m_mux_is_set != rhs_reg->m_mux_is_set) return false;

  if (m_mux_is_set) {
    return m_waddr_val == rhs_reg->m_waddr_val && m_mux_val == rhs_reg->m_mux_val;
  } else {
    return m_waddr_val == rhs_reg->m_waddr_val;
  }
}

}  // instr
}  // v3d
}  // V3DLib
