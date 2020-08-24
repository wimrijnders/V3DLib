#ifndef _QPULIB_V3D_INSTR_SMALLIMM_H
#define _QPULIB_V3D_INSTR_SMALLIMM_H
#include <stdint.h>
#include "broadcom/qpu/qpu_instr.h"
#include "dump_instr.h"

namespace QPULib {
namespace v3d {
namespace instr {


class SmallImm {
public:
	SmallImm(int val) : m_val(val) { pack(); }

	uint8_t to_raddr() const;
	v3d_qpu_input_unpack input_unpack() const { return m_input_unpack; }

	SmallImm l() const;

	static bool to_opcode_value(float value, int &rep_value);

private:
	int m_val = 0;
	uint8_t m_index = 0xff;  // init to illegal value
	v3d_qpu_input_unpack m_input_unpack = V3D_QPU_UNPACK_NONE;

	void pack();
};

}  // instr
}  // v3d
}  // QPULib

#endif  // _QPULIB_V3D_INSTR_SMALLIMM_H
