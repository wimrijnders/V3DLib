#include "SmallImm.h"
#include <stdio.h>
#include "Support/debug.h"

namespace QPULib {
namespace v3d {
namespace instr {

bool SmallImm::int_to_opcode_value(int value, int &rep_value) {
	bool converted  = true;

	// The first small values pass through as is
	// Note that this negates usage of next 4 if-s.
	if (-16 <= value && value <= 15) {
		rep_value = (int) value;
	} else {
		converted = false;
	}

	return converted;
}


bool SmallImm::float_to_opcode_value(float value, int &rep_value) {
	bool converted  = true;
	// NOTE: Apparently, these are the hex representations of the floats
  // TODO check this
	if      (value ==   1) rep_value = 0x3f800000; /* 2.0^0 */
	else if (value ==   2) rep_value = 0x40000000; /* 2.0^1 */
	else if (value ==   4) rep_value = 0x40800000; /* 2.0^2 */
	else if (value ==   8) rep_value = 0x41000000; /* 2.0^3 */
	else if (value ==  16) rep_value = 0x41800000; /* 2.0^4 */
	else if (value ==  32) rep_value = 0x42000000; /* 2.0^5 */
	else if (value ==  64) rep_value = 0x42800000; /* 2.0^6 */
	else if (value == 128) rep_value = 0x43000000; /* 2.0^7 */
	else if (value == 2e-8f) rep_value = 0x3b800000; /* 2.0^-8 */
	else if (value == 2e-7f) rep_value = 0x3c000000; /* 2.0^-7 */
	else if (value == 2e-6f) rep_value = 0x3c800000; /* 2.0^-6 */
	else if (value == 2e-5f) rep_value = 0x3d000000; /* 2.0^-5 */
	else if (value == 2e-4f) rep_value = 0x3d800000; /* 2.0^-4 */
	else if (value == 2e-3f) rep_value = 0x3e000000; /* 2.0^-3 */
	else if (value == 2e-2f) rep_value = 0x3e800000; /* 2.0^-2 */
	else if (value == 2e-1f) rep_value = 0x3f000000; /* 2.0^-1 */
	else converted = false;

	return converted;
}


uint8_t SmallImm::to_raddr() const {
	assert(m_index != 0xff);
	return m_index;
}


void SmallImm::pack() {
	uint32_t packed_small_immediate;

	if (small_imm_pack(m_val, &packed_small_immediate)) {
		assert(packed_small_immediate <= 0xff);  // to be sure conversion is OK
		m_index = (uint8_t) packed_small_immediate;
	} else {
		printf("SmallImm::pack(): Can not pack value %d\n", m_val);
		breakpoint
		assert(false);
	}
}


SmallImm SmallImm::l() const {
	SmallImm ret(*this);
	ret.m_input_unpack = V3D_QPU_UNPACK_L;
	return ret;
}


SmallImm SmallImm::ff() const {
	SmallImm ret(*this);
	ret.m_input_unpack = V3D_QPU_UNPACK_REPLICATE_32F_16;
	return ret;
}


}  // instr
}  // v3d
}  // QPULib
