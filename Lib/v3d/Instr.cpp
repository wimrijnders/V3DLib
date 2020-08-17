#include "Instr.h"
#include <cstdio>
#include <cstdlib>  // abs()
#include "../debug.h"
#include "dump_instr.h"

namespace {

struct v3d_device_info devinfo;  // NOTE: uninitialized struct, field 'ver' must be set! For asm/disasm OK


bool is_power_of_2(int x) {
    return x > 0 && !(x & (x - 1));
}

}


namespace QPULib {
namespace v3d {
namespace instr {

struct Exception : public std::exception {
   std::string s;
   Exception(std::string ss) : s(ss) {}
   ~Exception() throw () {} // Updated   const char* what() const throw() { return s.c_str(); }
};


///////////////////////////////////////////////////////////////////////////////
// Class SmallImm
///////////////////////////////////////////////////////////////////////////////

bool SmallImm::to_opcode_value(float value, int &rep_value) {
	bool converted  = true;

	// The first small values pass through as is
	// Note that this negates usage of next 4 if-s.
	if (-16 <= value && value <= 15) {
		rep_value = (int) value;
	}
	else if (value ==   1) rep_value = 0x3f800000; /* 2.0^0 */
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
		assert(false);
	}
}


SmallImm SmallImm::l() const {
	SmallImm ret(*this);
	ret.m_input_unpack = V3D_QPU_UNPACK_L;
	return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Class Register
///////////////////////////////////////////////////////////////////////////////

Register::Register(const char *name, v3d_qpu_waddr waddr_val) :
	m_name(name),
	m_waddr_val(waddr_val),
	m_mux_val(V3D_QPU_MUX_R0),  // Dummy value, set to first element in item
	m_mux_is_set(false)
{}


Register::Register(const char *name, v3d_qpu_waddr waddr_val, v3d_qpu_mux mux_val) :
	m_name(name),
	m_waddr_val(waddr_val),
	m_mux_val(mux_val),
	m_mux_is_set(true)
{}


v3d_qpu_mux Register::to_mux() const {
	if (!m_mux_is_set) {
		std::string buf;
		buf += "Can't use mux value for register '" + m_name + "', it is not defined";
		throw Exception(buf);
	}

	return m_mux_val;
}


Register const r0("r0", V3D_QPU_WADDR_R0, V3D_QPU_MUX_R0);
Register const r1("r1", V3D_QPU_WADDR_R1, V3D_QPU_MUX_R1);
Register const r2("r2", V3D_QPU_WADDR_R2, V3D_QPU_MUX_R2);
Register const r3("r3", V3D_QPU_WADDR_R3, V3D_QPU_MUX_R3);
Register const r4("r4", V3D_QPU_WADDR_R4, V3D_QPU_MUX_R4);
Register const r5("r5", V3D_QPU_WADDR_R5, V3D_QPU_MUX_R5);
Register const tmua("tmua", V3D_QPU_WADDR_TMUA);
Register const tmud("tmud", V3D_QPU_WADDR_TMUD);


///////////////////////////////////////////////////////////////////////////////
// Class RFAddress
///////////////////////////////////////////////////////////////////////////////

v3d_qpu_mux RFAddress::to_mux() const {
	debug_break("Not expecting RFAddress::to_mux() to be called");
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


///////////////////////////////////////////////////////////////////////////////
// Class Instr
///////////////////////////////////////////////////////////////////////////////

uint64_t const Instr::NOP = 0x3c003186bb800000;  // This is actually 'nop nop'


Instr::Instr(uint64_t in_code) {
	init(in_code);
}

std::string Instr::dump(bool to_stdout) const {
	std::string ret;
	char buffer[10*1024];
	instr_dump(buffer, const_cast<Instr *>(this));

	if (to_stdout) {
		printf("%llx:\n%s", code(), buffer);
	} else {
		ret = buffer;
	}

	return ret;
}


void Instr::dump_mnemonic() const {
	instr_dump_mnemonic(this);
	printf("\n");
}


uint64_t Instr::code() const {
	init_ver();

  uint64_t repack = instr_pack(&devinfo, const_cast<Instr *>(this));
	return repack;
}


void Instr::show(uint64_t in_code) {
	Instr instr(in_code);
	instr.dump(true);
}


void Instr::init_ver() const {
	if (devinfo.ver != 42) {               //        <-- only this needs to be set
		devinfo.ver = 42;
	}
}


void Instr::init(uint64_t in_code) {
	init_ver();

	raddr_a = 0;

	// These do not always get initialized in unpack
	sig_addr = 0;
	sig_magic = false;
	raddr_b = 0; // Not set for branch

	if (!instr_unpack(&devinfo, in_code, this)) {
		assert(false);
	}

	if (type == V3D_QPU_INSTR_TYPE_BRANCH) {
		if (!branch.ub) {
			// take over the value anyway
			branch.bdu = (v3d_qpu_branch_dest) ((in_code >> 15) & 0b111);
		}
	}

}


bool Instr::compare_codes(uint64_t code1, uint64_t code2) {
	if (code1 == code2) {
		return true;
	}

	// Here's the issue:
	// For the branch instruction, if field branch.ub != true, then field branch bdu is not used
	// and can have any value.
	// So for a truthful compare, in this special case the field needs to be ignored.
	// Determine if this is a branch
	auto is_branch = [] (uint64_t code) -> bool {
		uint64_t mul_op_mask = ((uint64_t) 0xb11111) << 58;
		bool is_mul_op = 0 != ((code & mul_op_mask) >> 58);

		uint64_t branch_sig_mask = ((uint64_t) 1) << 57;
		bool has_branch_sig = 0 != (code & branch_sig_mask) >> 57;

		return (!is_mul_op && has_branch_sig);
	};


	if (!is_branch(code1) || !is_branch(code2)) {
		return false;  // Not the special case we're looking for
	}

	// Check if bu flag is set; if so, ignore bdu field
	auto is_bu_set = [] (uint64_t code) -> bool {
		uint64_t bu_mask = ((uint64_t) 1) << 14;
		return 0 != (code & bu_mask);
	};

	if (is_bu_set(code1) || is_bu_set(code2)) {
		return false;  // bdu may be used
	}

	// Zap out the bdu field and compare again
	uint64_t bdu_mask = ((uint64_t) 0b111) << 15;
	code1 = code1 & ~bdu_mask;
	code2 = code1 & ~bdu_mask;

	return code1 == code2;
}


//////////////////////////////////////////////////////
// Calls to set the mult part of the instruction
//////////////////////////////////////////////////////

void Instr::set_c(v3d_qpu_cond val) {
	if (m_doing_add) {
		flags.ac = val;
	} else {
		flags.mc = val;
	}
}


void Instr::set_uf(v3d_qpu_uf val) {
	if (m_doing_add) {
		flags.auf = val;
	} else {
		flags.muf = val;
	}
}


void Instr::set_pf(v3d_qpu_pf val) {
	if (m_doing_add) {
		flags.apf = val;
	} else {
		flags.mpf = val;
	}
}


Instr &Instr::pushc() { set_pf(V3D_QPU_PF_PUSHC); return *this; }
Instr &Instr::pushn() { set_pf(V3D_QPU_PF_PUSHN); return *this; }
Instr &Instr::pushz() { set_pf(V3D_QPU_PF_PUSHZ); return *this; }
Instr &Instr::norc()  { set_uf(V3D_QPU_UF_NORC);  return *this; }
Instr &Instr::nornc() { set_uf(V3D_QPU_UF_NORNC); return *this; }
Instr &Instr::norz()  { set_uf(V3D_QPU_UF_NORZ);  return *this; }
Instr &Instr::nornn() { set_uf(V3D_QPU_UF_NORNN); return *this; }
Instr &Instr::andnc() { set_uf(V3D_QPU_UF_ANDNC); return *this; }
Instr &Instr::andnn() { set_uf(V3D_QPU_UF_ANDNN); return *this; }
Instr &Instr::ifnb()  { set_c(V3D_QPU_COND_IFNB); return *this; }
Instr &Instr::ifb()   { set_c(V3D_QPU_COND_IFB);  return *this; }
Instr &Instr::ifna()  { set_c(V3D_QPU_COND_IFNA); return *this; }
Instr &Instr::ifa()   { set_c(V3D_QPU_COND_IFA);  return *this; }


Instr &Instr::thrsw(bool val) {
	sig.thrsw = val;
	return *this;
}


// TODO: Where does reg go??
Instr &Instr::ldtmu(Register const &reg) {
	sig.ldtmu = true;
	sig_addr  = reg.to_waddr(); 

	return *this;
}


Instr &Instr::ldvary(bool val) {
	sig.ldvary = val;
	return *this;
}


Instr &Instr::ldunif(bool val) {
	sig.ldunif = val;
	return *this;
}


Instr &Instr::anyap() {
	assert(type == V3D_QPU_INSTR_TYPE_BRANCH);  // Branch instruction-specific
	branch.cond = V3D_QPU_BRANCH_COND_ANYA;
	return *this;
}


Instr &Instr::ldunifa(bool val) {
	sig.ldunifa = val;
	return *this;
}


Instr &Instr::ldvpm(bool val) {
	sig.ldvpm = val;
	return *this;
}


Instr &Instr::cond_na0() {
	branch.cond = V3D_QPU_BRANCH_COND_NA0;
	return *this;
}


Instr &Instr::add(uint8_t  rf_addr1, uint8_t rf_addr2, Register const &reg3) {
	m_doing_add = false;

	raddr_b       = rf_addr1; 
	alu.mul.op    = V3D_QPU_M_ADD;
	alu.mul.a     = V3D_QPU_MUX_B;
	alu.mul.b     = reg3.to_mux();
	alu.mul.waddr = rf_addr2;
	alu.mul.magic_write = false;

	return *this;
}


// TODO: where does 1 go??
Instr &Instr::add(uint8_t rf_addr1, uint8_t rf_addr2, uint8_t rf_addr3) {
	m_doing_add = false;

	raddr_b       = rf_addr3; 
	alu.mul.op    = V3D_QPU_M_ADD;
	alu.mul.a     = V3D_QPU_MUX_A;
	alu.mul.b     = V3D_QPU_MUX_B;
	alu.mul.waddr = rf_addr2;
	alu.mul.magic_write = false;

	return *this;
}


Instr &Instr::sub(uint8_t rf_addr1, uint8_t rf_addr2, Register const &reg3) {
	m_doing_add = false;

	raddr_b       = rf_addr1; 
	alu.mul.op    = V3D_QPU_M_SUB;
	alu.mul.a     = V3D_QPU_MUX_B;
	alu.mul.b     = reg3.to_mux();
	alu.mul.waddr = rf_addr2;
	alu.mul.magic_write = false;

	return *this;
}


Instr &Instr::mov(Register const &reg,  uint8_t val) {
	m_doing_add = false;

	alu.mul.op    = V3D_QPU_M_MOV;
	alu.mul.a     = V3D_QPU_MUX_B;
	alu.mul.b     = V3D_QPU_MUX_B;
	alu.mul.waddr = reg.to_waddr();

	return *this;
}


Instr &Instr::mov(uint8_t rf_addr, Register const &reg) {
	m_doing_add = false;

	alu.mul.op    = V3D_QPU_M_MOV;
	alu.mul.a     = reg.to_mux();
	alu.mul.b     = V3D_QPU_MUX_B;
	alu.mul.waddr = rf_addr;

	return *this;
}


Instr &Instr::fmul(Location const &loc1, Location const &loc2, Location const &loc3) {
	m_doing_add = false;
	alu_mul_set(loc1, loc2, loc3);

	alu.mul.op    = V3D_QPU_M_FMUL;

	return *this;
}


// TODO: how does small imm value get used?
Instr &Instr::fmul(Location const &loc1, SmallImm imm2, Location const &loc3) {
	m_doing_add = false;

	sig.small_imm = true;
	raddr_a = loc3.to_waddr();  // Or this:  abs(imm2);  // TODO: so how can you use a negative value?
	alu.mul.op    = V3D_QPU_M_FMUL;
	alu.mul.a     = V3D_QPU_MUX_B;
	alu.mul.b     = V3D_QPU_MUX_A;
	alu.mul.waddr = loc1.to_waddr();
	alu.mul.magic_write = false;
	alu.mul.b_unpack = loc3.input_unpack();

	return *this;
}


Instr &Instr::smul24(Location const &loc1, Location const &loc2, Location const &loc3) {
	m_doing_add = false;
	alu_mul_set(loc1, loc2, loc3);

	sig.small_imm = true;
	raddr_b = loc1.to_waddr();
	alu.mul.op    = V3D_QPU_M_SMUL24;

	return *this;
}


Instr &Instr::vfmul(Location const &rf_addr1, Register const &reg2, Register const &reg3) {
	m_doing_add = false;

	alu_mul_set(rf_addr1, reg2, reg3);

	alu.mul.op    = V3D_QPU_M_VFMUL;

	return *this;
}


void Instr::alu_add_set(Location const &loc1, Location const &loc2, Location const &loc3) {
	if (loc1.is_rf()) {
		alu.add.magic_write = false;
	} else {
		alu.add.magic_write = true;
	}

	if (loc2.is_rf()) {
		raddr_a = loc2.to_waddr();
		alu.add.a     = V3D_QPU_MUX_A;
	} else {
		alu.add.a     = loc2.to_mux();
	}

	if (loc3.is_rf()) {
		raddr_b          = loc3.to_waddr(); 
		alu.add.b        = V3D_QPU_MUX_B;
	} else {
		alu.add.b        = loc3.to_mux();
		alu.add.b_unpack = loc3.input_unpack();
	}

	alu.add.waddr = loc1.to_waddr();
	alu.add.output_pack = loc1.output_pack();
	alu.add.a_unpack = loc2.input_unpack();
}


void Instr::alu_mul_set(Location const &loc1, Location const &loc2, Location const &loc3) {
	if (loc1.is_rf()) {
		alu.mul.magic_write = false;
	} else {
		alu.mul.magic_write = true;
	}

	if (loc2.is_rf()) {
		raddr_a = loc2.to_waddr();
		alu.mul.a     = V3D_QPU_MUX_A;
	} else {
		alu.mul.a     = loc2.to_mux();
	}

	alu.mul.b     = loc3.to_mux();
	alu.mul.waddr = loc1.to_waddr();
	alu.mul.output_pack = loc1.output_pack();
	alu.mul.a_unpack = loc2.input_unpack();
	alu.mul.b_unpack = loc3.input_unpack();
}


//////////////////////////////////////////////////////
// Top-level opcodes
//////////////////////////////////////////////////////

Instr nop() {
	Instr instr;
	return instr;
}


Instr ldunifrf(uint8_t rf_address) {
	Instr instr;

	//instr.sig_magic    = false; 
	instr.sig.ldunifrf = true; 
	instr.sig_addr = rf_address; 

	return instr;
}


Instr shr(Register const &reg1, Register const &reg2, uint8_t val) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.sig_magic     = true; 
	instr.raddr_a       = reg2.to_waddr(); 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_SHR;
	instr.alu.add.waddr = reg1.to_waddr();
	instr.alu.add.b     = V3D_QPU_MUX_B;

	return instr;
}


Instr shr(uint8_t rf_addr1, uint8_t rf_addr2, int val) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.sig_magic     = true; 
	instr.raddr_a       = rf_addr1; 
	instr.raddr_b       = (uint8_t) val; 
	instr.alu.add.op    = V3D_QPU_A_SHR;
	instr.alu.add.waddr = rf_addr2;
	instr.alu.add.magic_write = false;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = V3D_QPU_MUX_B;

	return instr;
}


Instr shl(Location const &loc1, Location const &loc2, SmallImm val) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.raddr_a       = loc1.to_waddr(); 
	instr.raddr_b       = val.to_raddr(); 
	instr.alu.add.op    = V3D_QPU_A_SHL;
	instr.alu.add.a     = loc2.to_mux();
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = loc1.to_waddr();

	return instr;
}


Instr shl(Register const &reg1, uint8_t rf_addr, uint8_t val) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.raddr_a       = rf_addr; 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_SHL;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = reg1.to_waddr();

	return instr;
}


Instr shl(uint8_t rf_addr1, uint8_t rf_addr2, int  val) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.raddr_a       = rf_addr1; 
	instr.raddr_b       = (uint8_t) val; 
	instr.sig_magic     = true; 
	instr.alu.add.op    = V3D_QPU_A_SHL;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = rf_addr2;
	instr.alu.add.magic_write = false;

	return instr;
}

// 'and' is a keyword
Instr band(uint8_t rf_address, Register const &reg, uint8_t val) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_AND;
	instr.alu.add.waddr = rf_address;
	instr.alu.add.a     = reg.to_mux();
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.magic_write = false;

	return instr;
}


/**
 * Returns index of current vector itemi on a given QPU.
 * This will be something in the range [0..15]
 */
Instr eidx(Register const &reg) {
	Instr instr;

	instr.alu.add.op    = V3D_QPU_A_EIDX;
	instr.alu.add.waddr = reg.to_waddr();
	instr.alu.add.a     = V3D_QPU_MUX_R2;
	instr.alu.add.b     = V3D_QPU_MUX_R0;

	return instr;
}


Instr tidx(Register const &reg) {
	Instr instr;

	instr.sig_magic  = true; 
	instr.alu.add.op = V3D_QPU_A_TIDX;
	instr.alu.add.waddr = reg.to_waddr();
	instr.alu.add.a  = V3D_QPU_MUX_R1;
	instr.alu.add.b  = V3D_QPU_MUX_R0;

	return instr;
}


Instr add(Location const &loc1, Location const &loc2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, loc3);


	instr.alu.add.op    = V3D_QPU_A_ADD;
	return instr;
}


// TODO: consolidate with first implementation
Instr add(uint8_t rf_addr1, uint8_t rf_addr2, Register const &reg3) {
	// Following is the goal, does not work yet (hangs the QPU)
	//return add(rf(rf_addr1), rf(rf_addr2), reg3);

	Instr instr;

	instr.raddr_a       = rf_addr1; 
	instr.sig_magic     = true;
	instr.alu.add.op    = V3D_QPU_A_ADD;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = reg3.to_mux();
	instr.alu.add.waddr = rf_addr2;
	instr.alu.add.magic_write = false;

	return instr;
}


// TODO: consolidate with first implementation
Instr add(uint8_t rf_addr1, uint8_t rf_addr2, uint8_t rf_addr3) {
	//printf("add() called addr1: %x,  addr3: %x\n", rf_addr1, rf_addr3);
	Instr instr;

	instr.raddr_a       = rf_addr1; 
	instr.raddr_b       = rf_addr3; 
	instr.sig_magic     = true;
	instr.alu.add.op    = V3D_QPU_A_ADD;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = rf_addr2;
	instr.alu.add.magic_write = false;

	return instr;
}


Instr fadd(Location const &loc1, Location const &loc2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, loc3);

	instr.alu.add.op    = V3D_QPU_A_FADD;
	return instr;
}


Instr mov(Location const &loc1, SmallImm val) {
	Instr instr;

	// hypothesis: magic_write true selects register, false address in register file 
	if (loc1.is_rf()) {
		instr.alu.add.magic_write = false;
	} else {
		instr.alu.add.magic_write = true;
	}

	instr.sig.small_imm = true; 

//	instr.raddr_a       = loc1.to_waddr(); 
	instr.raddr_b       = val.to_raddr(); 
	instr.alu.add.op    = V3D_QPU_A_OR;
	instr.alu.add.a     = V3D_QPU_MUX_B; // loc2.to_mux();
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = loc1.to_waddr();

	return instr;
}


/**
 * Location for loc2 hangs the GPU in unit tests.
 * TODO: examine and fix
 */
Instr mov(Register const &reg, RFAddress /* Location */ const &loc2) {
	Instr instr;

	if (loc2.is_rf()) {
		instr.raddr_a = loc2.to_waddr();
		instr.alu.add.a     = V3D_QPU_MUX_A;
	} else {
		instr.alu.add.a     = loc2.to_mux();
	}
	//instr.raddr_a       = addr.to_waddr(); 
	//instr.alu.add.a     = V3D_QPU_MUX_A;

	instr.alu.add.op    = V3D_QPU_A_OR;
	instr.alu.add.b     = V3D_QPU_MUX_A;
	instr.alu.add.waddr = reg.to_waddr();
	instr.alu.add.magic_write = true;

	return instr;
}


// or is reserved keyword
Instr bor(Location const &loc1, Location const &loc2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, loc3);

	instr.alu.add.op    = V3D_QPU_A_OR;

	return instr;
}


Instr bxor(uint8_t rf_addr, uint8_t val1, uint8_t val2) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.raddr_b       = val1; 
	instr.alu.add.op    = V3D_QPU_A_XOR;
	instr.alu.add.a     = V3D_QPU_MUX_B;
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = rf_addr;
	instr.alu.add.magic_write = false;

	return instr;
}


/**
 * NOTE: needs condition set to work!
 *       eg. `cond na0`
 */
Instr branch(int target, int current) {
	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	instr.branch.msfign = V3D_QPU_MSFIGN_NONE;
	instr.branch.bdi = V3D_QPU_BRANCH_DEST_REL;  // branch dest
	instr.branch.bdu = V3D_QPU_BRANCH_DEST_REL;  // not used when branch.ub == false, just set a value for now
	instr.branch.ub = false;
	instr.branch.raddr_a = 0;

	// branch needs 4 delay slots before executing, hence the 4
	// This means that 3 more instructions will execute after the loop before jumping
	instr.branch.offset = (unsigned) 8*(target - (current + 4));

	return instr;
}


/**
 * Actually called just 'b' in the mnemonics
 */
Instr bb(Location const &loc1) {
	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

  instr.sig.ldunif = true;
  instr.sig.ldunifa = true;

	// flags: {ac: <<UNKNOWN>>, mc: <<UNKNOWN>>, apf: <<UNKNOWN>>, mpf: PF_PUSHZ, auf: UF_NONE, muf: <<UNKNOWN>>},
	instr.flags.mpf = V3D_QPU_PF_PUSHZ;
	instr.flags.auf = V3D_QPU_UF_NONE;

//	instr.branch.cond = V3D_QPU_BRANCH_COND_ANYA;
	instr.branch.msfign =  V3D_QPU_MSFIGN_P;
	instr.branch.bdi =  V3D_QPU_BRANCH_DEST_REGFILE;
	instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
	instr.branch.ub =  false;
	instr.branch.raddr_a = loc1.to_waddr();
	instr.branch.offset = 0;

	return instr;
}


Instr barrierid(v3d_qpu_waddr waddr) {
	Instr instr;

	instr.alu.add.op    = V3D_QPU_A_BARRIERID;
	instr.alu.add.a     = V3D_QPU_MUX_R4;
	instr.alu.add.b     = V3D_QPU_MUX_R2;
	instr.alu.add.waddr = waddr;

	return instr;
}


Instr vpmsetup(Register const &reg2) {
	Instr instr;

	instr.alu.add.op    = V3D_QPU_A_VPMSETUP;
	instr.alu.add.a     = reg2.to_mux();
	instr.alu.add.b     = V3D_QPU_MUX_R3;

	return instr;
}


// First param ignored??
Instr ffloor(uint32_t magic_value, RFAddress rf_addr2, Register const &reg3) {
	Instr instr;

	if (magic_value == ifb) {
		instr.flags.ac = V3D_QPU_COND_IFB;
	}


	instr.alu.add.op    = V3D_QPU_A_FFLOOR;
	instr.alu.add.a     = reg3.to_mux();
	instr.alu.add.b     = V3D_QPU_MUX_R1;
	instr.alu.add.waddr = rf_addr2.to_waddr();
	instr.alu.add.magic_write = false;
	instr.alu.add.output_pack = rf_addr2.output_pack();
	instr.alu.add.b_unpack = (v3d_qpu_input_unpack) magic_value;

	return instr;
}


Instr flpop(RFAddress rf_addr1, RFAddress rf_addr2) {
	Instr instr;

	instr.raddr_a       = rf_addr2.to_waddr();
	instr.alu.add.op    = V3D_QPU_A_FLPOP;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = V3D_QPU_MUX_R4;
	instr.alu.add.waddr = rf_addr1.to_waddr();
	instr.alu.add.magic_write = false;

	return instr;
}


Instr fmax(Location const &rf_addr1, Location const &reg2, Location const &reg3) {
	Instr instr;
	instr.alu_add_set(rf_addr1, reg2, reg3);

	instr.alu.add.op    = V3D_QPU_A_FMAX;

	return instr;
}


Instr faddnf(Location const &loc1, Location const &reg2, Location const &reg3) {
	Instr instr;
	instr.alu_add_set(loc1, reg2, reg3);

	instr.alu.add.op    = V3D_QPU_A_FADDNF;

	return instr;
}


Instr fcmp(Location const &loc1, Location const &reg2, Location const &reg3) {
	Instr instr;
	instr.alu_add_set(loc1, reg2, reg3);

	instr.alu.add.op    = V3D_QPU_A_FCMP;

	return instr;
}


Instr fsub(Location const &loc1, Location const &reg2, Location const &reg3) {
	Instr instr;
	instr.alu_add_set(loc1, reg2, reg3);

	instr.alu.add.op    = V3D_QPU_A_FSUB;

	return instr;
}


Instr vfpack(Location const &loc1, Location const &loc2, Location const &loc3) {
	Instr instr;
	//Can't use here: instr.alu_add_set(loc1, loc2, loc3);

	instr.raddr_a       = loc2.to_waddr();
	instr.alu.add.op    = V3D_QPU_A_VFPACK;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = loc3.to_mux();
	instr.alu.add.waddr = loc1.to_waddr();
	instr.alu.add.magic_write = false;
	instr.alu.add.a_unpack = loc2.input_unpack();
	instr.alu.add.b_unpack = loc3.input_unpack();

	return instr;
}


Instr fdx(Location const &loc1, Location const &loc2) {
	Instr instr;

	instr.alu.add.op    = V3D_QPU_A_FDX;
	instr.alu.add.a     = loc2.to_mux();
	instr.alu.add.b     = V3D_QPU_MUX_R2;
	instr.alu.add.waddr = loc1.to_waddr();
	instr.alu.add.magic_write = false;
	instr.alu.add.output_pack = loc1.output_pack();
	instr.alu.add.a_unpack = loc2.input_unpack();

	return instr;
}


Instr vflb(Location const &loc1) {
	Instr instr;

	instr.raddr_b       = loc1.to_waddr();
	instr.alu.add.op    = V3D_QPU_A_VFLB;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = V3D_QPU_MUX_R0;
	instr.alu.add.waddr = loc1.to_waddr();
	instr.alu.add.magic_write = false;

	return instr;
}


Instr vfmin(Location const &loc1, SmallImm imm2, Location const &loc3) {
	Instr instr;

	instr.sig.small_imm = true;
	instr.raddr_b = imm2.to_raddr();
	instr.alu.add.op    = V3D_QPU_A_VFMIN;
	instr.alu.add.a     = V3D_QPU_MUX_B;
	instr.alu.add.b     = loc3.to_mux();
	instr.alu.add.magic_write = false;
	instr.alu.add.waddr = loc1.to_waddr();
	instr.alu.add.a_unpack = imm2.input_unpack();
	instr.alu.add.a_unpack = V3D_QPU_UNPACK_REPLICATE_32F_16;

	return instr;
}


Instr faddnf(Location const &loc1, SmallImm imm2, Location const &loc3) {
	Instr instr;

	instr.sig.small_imm = true;
	instr.raddr_b = imm2.to_raddr();
	instr.alu.add.op    = V3D_QPU_A_FADDNF;
	instr.alu.add.a     = V3D_QPU_MUX_B;
	instr.alu.add.b     = loc3.to_mux();
	instr.alu.add.magic_write = false;
	instr.alu.add.waddr = loc1.to_waddr();
	instr.alu.add.a_unpack = imm2.input_unpack();
	instr.alu.add.b_unpack = loc3.input_unpack();

/*
	instr.alu.add.a_unpack = V3D_QPU_UNPACK_REPLICATE_32F_16;
*/

	return instr;
}

}  // instr
}  // v3d
}  // QPULib
