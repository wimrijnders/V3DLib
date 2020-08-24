#ifndef _QPULIB_V3D_INSTR_LOCATION_H
#define _QPULIB_V3D_INSTR_LOCATION_H
#include "broadcom/qpu/qpu_instr.h"

namespace QPULib {
namespace v3d {
namespace instr {

class Location {
public:
	virtual v3d_qpu_waddr to_waddr() const = 0;
	virtual v3d_qpu_mux to_mux() const = 0;

	v3d_qpu_output_pack output_pack() const { return m_output_pack; }
	v3d_qpu_input_unpack input_unpack() const { return m_input_unpack; }
	bool is_rf() const { return m_is_rf; }

protected:
	v3d_qpu_output_pack m_output_pack = V3D_QPU_PACK_NONE;
	v3d_qpu_input_unpack m_input_unpack = V3D_QPU_UNPACK_NONE;
	bool m_is_rf = false;
};

}  // instr
}  // v3d
}  // QPULib

#endif  // _QPULIB_V3D_INSTR_LOCATION_H
