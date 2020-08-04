#ifndef _QPULIB_V3D_INSTR_H
#define _QPULIB_V3D_INSTR_H
#include <cstdint>
#include <string>
#include "dump_instr.h"

namespace QPULib {
namespace v3d {
namespace instr {

class Register {
public: 
	Register(const char *name, v3d_qpu_waddr waddr_val);
	Register(const char *name, v3d_qpu_waddr waddr_val, v3d_qpu_mux mux_val);

	v3d_qpu_waddr to_waddr() const { return m_waddr_val; }
	v3d_qpu_mux to_mux() const;

private:
	std::string   m_name;
	v3d_qpu_waddr m_waddr_val;
	v3d_qpu_mux   m_mux_val;
	bool          m_mux_is_set = false;
};


class Instr : public v3d_qpu_instr {
public:
	Instr(uint64_t code = NOP) { init(code); }

	std::string dump(bool to_stdout = false) const; 
	uint64_t code() const;
	static void show(uint64_t code);

	operator uint64_t() const { return code(); }

	Instr &thrsw(bool val) { sig.thrsw = val; return *this; }
	Instr &pushz();
	Instr &ldtmu(Register const &reg);

	// Calls to set the mul part of the instruction
	Instr &add(uint8_t rf_addr1, uint8_t rf_addr2, Register const &reg3);
	Instr &add(uint8_t rf_addr1, uint8_t rf_addr2, uint8_t rf_addr3);
	Instr &sub(uint8_t rf_addr1, uint8_t rf_addr2, Register const &reg3);

	Instr &mov(Register const &reg, uint8_t val);

private:
	static uint64_t const NOP;

	void init_ver() const;
	void init(uint64_t code);
};


extern Register const r0;
extern Register const r1;
extern Register const tmua;

Instr nop();
Instr ldunifrf(uint8_t rf_address);
Instr tidx(Register const &reg);

Instr shr(Register const &reg1, Register const &reg2, uint8_t val);
Instr shr(uint8_t rf_addr1, uint8_t rf_addr2, int val);

Instr shl(Register const &reg1, Register const & reg2, uint8_t val);
Instr shl(Register const &reg1, uint8_t rf_addr, uint8_t val);
Instr shl(uint8_t rf_addr1, uint8_t rf_addr2, int val);

Instr band(uint8_t rf_address, Register const &reg, uint8_t val);
Instr eidx(Register const &reg);

Instr add(Register const &reg1, Register const &reg2, Register const &reg3);
Instr add(uint8_t rf_addr1, uint8_t rf_addr2, Register const &reg3);
Instr add(uint8_t rf_addr1, uint8_t rf_addr2, uint8_t ref_addr3);

Instr mov(uint8_t rf_addr, uint8_t val);
Instr mov(Register const &reg, uint8_t rf_addr);


Instr bxor(uint8_t rf_addr, uint8_t val1, uint8_t val2);

}  // instr
}  // v3d
}  // QPULib


#endif // _QPULIB_V3D_INSTR_H
