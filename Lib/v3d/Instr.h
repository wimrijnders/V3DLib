#ifndef _QPULIB_V3D_INSTR_H
#define _QPULIB_V3D_INSTR_H
#include <cstdint>
#include <string>
#include <vector>
#include "dump_instr.h"

namespace QPULib {
namespace v3d {
namespace instr {

class Register {
public: 
	Register(const char *name, v3d_qpu_waddr waddr_val);
	Register(const char *name, v3d_qpu_waddr waddr_val, v3d_qpu_mux mux_val);
//	Register(Register const &rhs) = default;

	v3d_qpu_waddr to_waddr() const { return m_waddr_val; }
	v3d_qpu_mux to_mux() const;
	v3d_qpu_input_unpack input_unpack() const { return m_input_unpack; }

	Register l() const {
		Register ret(*this);
		ret.m_input_unpack = V3D_QPU_UNPACK_L;
		return ret;
	}

	Register h() const {
		Register ret(*this);
		ret.m_input_unpack = V3D_QPU_UNPACK_H;
		return ret;
	}

	Register abs() const {
		Register ret(*this);
		ret.m_input_unpack = V3D_QPU_UNPACK_ABS;
		return ret;
	}

private:
	std::string   m_name;
	v3d_qpu_waddr m_waddr_val;
	v3d_qpu_mux   m_mux_val;
	bool          m_mux_is_set = false;
	v3d_qpu_input_unpack m_input_unpack = V3D_QPU_UNPACK_NONE;
};


class RFAddress {
public:
	RFAddress(uint8_t val) : m_val(val) {}

	uint8_t to_waddr() const { return m_val; }
	v3d_qpu_output_pack output_pack() const { return m_output_pack; }

	RFAddress l() const {
		RFAddress ret(m_val);
		ret.m_output_pack = V3D_QPU_PACK_L;

		return ret;
	}

	RFAddress h() const {
		RFAddress ret(m_val);
		ret.m_output_pack = V3D_QPU_PACK_H;

		return ret;
	}

private:
	uint8_t m_val;
	v3d_qpu_output_pack m_output_pack = V3D_QPU_PACK_NONE;
};

using rf = RFAddress;


class Instr : public v3d_qpu_instr {
public:
	Instr(uint64_t in_code = NOP);

	std::string dump(bool to_stdout = false) const; 
	uint64_t code() const;
	static void show(uint64_t in_code);

	operator uint64_t() const { return code(); }

	Instr &thrsw(bool val = true);
	Instr &pushz();
	Instr &ldtmu(Register const &reg);
	Instr &ldvary(bool val = true);
	Instr &ldunif(bool val = true);
	Instr &ldunifa(bool val = true);
	Instr &ldvpm(bool val = true);
	Instr &nornn(bool val = true);
	Instr &ifnb(bool val = true);

	// Calls to set the mul part of the instruction
	Instr &add(uint8_t rf_addr1, uint8_t rf_addr2, Register const &reg3);
	Instr &add(uint8_t rf_addr1, uint8_t rf_addr2, uint8_t rf_addr3);
	Instr &sub(uint8_t rf_addr1, uint8_t rf_addr2, Register const &reg3);

	Instr &mov(Register const &reg, uint8_t val);
	Instr &mov(uint8_t rf_addr, Register const &reg);

	Instr &fmul(RFAddress rf_addr1, Register const &reg2, Register const &reg3);
	Instr &vfmul(RFAddress rf_addr1, Register const &reg2, Register const &reg3);

	static bool compare_codes(uint64_t code1, uint64_t code2);

	void alu_add_set(RFAddress rf_addr1, Register const &reg2, Register const &reg3); 
	void alu_mul_set(RFAddress rf_addr1, Register const &reg2, Register const &reg3); 

private:
	static uint64_t const NOP;

	void init_ver() const;
	void init(uint64_t in_code);

};


extern Register const r0;
extern Register const r1;
extern Register const r2;
extern Register const r3;
extern Register const r4;
extern Register const r5;
extern Register const tmua;
extern Register const tmud;
const uint8_t vpm = 14;
const uint32_t ifb = 3145728000;  // Value according to dump; No idea what this value is supposed to be and what it does!

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

Instr fadd(Register const &reg1, Register const &reg2, Register const &reg3);

Instr mov(uint8_t rf_addr, uint8_t val);
Instr mov(Register const &reg, uint8_t rf_addr);

Instr bor(uint8_t rf_addr1, Register const &reg2, Register const &reg3);
Instr bxor(uint8_t rf_addr, uint8_t val1, uint8_t val2);

Instr branch(int target, int current);

v3d_qpu_waddr const syncb = V3D_QPU_WADDR_SYNCB;

Instr barrierid(v3d_qpu_waddr waddr);
Instr vpmsetup(Register const &reg2);

Instr ffloor(uint32_t magic_value, RFAddress rf_addr2, Register const &reg3);
Instr flpop(RFAddress rf_addr1, RFAddress rf_addr2);
Instr fmax(RFAddress rf_addr1, Register const &reg2, Register const &reg3);

}  // instr
}  // v3d
}  // QPULib


// Useful definitions for outputting opcodes
// (pretty generic, though)
using Vec = std::vector<uint64_t>;

inline Vec &operator<<(Vec &a, uint64_t val) {
	a.push_back(val);	
	return a;
}


inline Vec &operator<<(Vec &a, Vec const &b) {
	a.insert(a.end(), b.begin(), b.end());
	return a;
}

#endif // _QPULIB_V3D_INSTR_H
