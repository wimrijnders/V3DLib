#ifndef _QPULIB_V3D_INSTR_H
#define _QPULIB_V3D_INSTR_H
#include <cstdint>
#include "dump_instr.h"

namespace QPULib {
namespace v3d {
namespace instr {

class Instr : public v3d_qpu_instr {
public:
	Instr(uint64_t code) { init(code); }

	void dump() const; 
	uint64_t code() const;
	static void show(uint64_t code);

	operator uint64_t() const { return code(); }

private:
	void init_ver() const;
	void init(uint64_t code);
};


class Register {
public: 
	Register(v3d_qpu_waddr waddr_val, v3d_qpu_mux mux_val) :
		m_waddr_val(waddr_val),
		m_mux_val(mux_val)
	{}

	operator v3d_qpu_waddr() const { return m_waddr_val; }
	operator v3d_qpu_mux() const { return m_mux_val; }

private:
	v3d_qpu_waddr m_waddr_val;
	v3d_qpu_mux   m_mux_val;
};


extern Register const r0;
extern Register const r1;
extern Instr const nop;

Instr ldunifrf(uint8_t rf_address);
Instr tidx(v3d_qpu_waddr reg);
Instr shr(v3d_qpu_waddr reg1, v3d_qpu_waddr reg2, uint8_t val);
Instr shl(v3d_qpu_waddr reg1, uint8_t rf_addr, uint8_t val);
Instr band(uint8_t rf_address, v3d_qpu_mux reg, uint8_t val);
Instr eidx(v3d_qpu_waddr reg);

}  // instr
}  // v3d
}  // QPULib


#endif // _QPULIB_V3D_INSTR_H
