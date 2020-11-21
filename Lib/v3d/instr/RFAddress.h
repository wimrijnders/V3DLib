#ifndef _V3DLIB_V3D_INSTR_RFADDRESS_H
#define _V3DLIB_V3D_INSTR_RFADDRESS_H
#include "Location.h"

namespace V3DLib {
namespace v3d {
namespace instr {

class RFAddress : public Location {
public:
	RFAddress(uint8_t val) : m_val(val) { m_is_rf = true; }

	v3d_qpu_waddr to_waddr() const override { return (v3d_qpu_waddr) m_val; }
	v3d_qpu_mux to_mux() const override;

	RFAddress l() const;
	RFAddress h() const;
	RFAddress abs() const;

private:
	uint8_t m_val;
};

}  // instr
}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_RFADDRESS_H
