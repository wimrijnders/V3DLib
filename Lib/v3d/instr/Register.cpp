#include "Register.h"
#include "../../Support/basics.h"  // Exception


namespace QPULib {
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


Register const r0("r0", V3D_QPU_WADDR_R0, V3D_QPU_MUX_R0, true);
Register const r1("r1", V3D_QPU_WADDR_R1, V3D_QPU_MUX_R1, true);
Register const r2("r2", V3D_QPU_WADDR_R2, V3D_QPU_MUX_R2, true);
Register const r3("r3", V3D_QPU_WADDR_R3, V3D_QPU_MUX_R3, true);
Register const r4("r4", V3D_QPU_WADDR_R4, V3D_QPU_MUX_R4, true);
Register const r5("r5", V3D_QPU_WADDR_R5, V3D_QPU_MUX_R5);
Register const tmua("tmua", V3D_QPU_WADDR_TMUA);
Register const tmud("tmud", V3D_QPU_WADDR_TMUD);

// For branch
BranchDest const lri("lri", V3D_QPU_WADDR_R0);

}  // instr
}  // v3d
}  // QPULib
