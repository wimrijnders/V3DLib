#ifndef _QPULIB_V3D_INSTR_RFADDRESS_H
#define _QPULIB_V3D_INSTR_RFADDRESS_H
#include "Location.h"

namespace QPULib {
namespace v3d {
namespace instr {

class RFAddress : public Location {
public:
	RFAddress(uint8_t val) : m_val(val) { m_is_rf = true; }

	v3d_qpu_waddr to_waddr() const override { return (v3d_qpu_waddr) m_val; }
	v3d_qpu_mux to_mux() const override;

	RFAddress l() const;
	RFAddress h() const;

private:
	uint8_t m_val;
};

}  // instr
}  // v3d
}  // QPULib

#endif  // _QPULIB_V3D_INSTR_RFADDRESS_H
