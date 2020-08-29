#include "RFAddress.h"
#include "../../Support/debug.h"

namespace QPULib {
namespace v3d {
namespace instr {


v3d_qpu_mux RFAddress::to_mux() const {
	debug /* _break */ ("Not expecting RFAddress::to_mux() to be called");
	//assert(false);
	return (v3d_qpu_mux) 0;
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


}  // instr
}  // v3d
}  // QPULib
