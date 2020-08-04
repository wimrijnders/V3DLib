#include "Instr.h"
#include <cstdio>

namespace {

struct v3d_device_info devinfo;  // NOTE: uninitialized struct, field 'ver' must be set! For asm/disasm OK


bool is_power_of_2(int x) {
    return x > 0 && !(x & (x - 1));
}

}  // anon namespace


namespace QPULib {
namespace v3d {
namespace instr {

struct Exception : public std::exception {
   std::string s;
   Exception(std::string ss) : s(ss) {}
   ~Exception() throw () {} // Updated
   const char* what() const throw() { return s.c_str(); }
};


///////////////////////////////////////////////////////////////////////////////
// Class Instr
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
Register const tmua("tmua", V3D_QPU_WADDR_TMUA);
Register const tmud("tmud", V3D_QPU_WADDR_TMUD);


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


uint64_t Instr::code() const {
	init_ver();

  uint64_t repack = instr_pack(&devinfo, const_cast<Instr *>(this));

	if (type == V3D_QPU_INSTR_TYPE_BRANCH) {
		if (!branch.ub) {
			// take over the value anyway
			uint64_t mask = 0b111 << 15;
			//repack = (repack & ~mask) | (((uint64_t) branch.bdu) & 0b111) << 15;
			repack = (repack & ~mask) | ((((uint64_t) branch.bdu) << 15) & mask);
		}
	}
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

	// These do not always get initialized in unpack
	sig_addr = 0;
	sig_magic = false;

	// Not set for branch
	raddr_b = 0; 
//	branch.ub  = false;
//	branch.bdu = V3D_QPU_BRANCH_DEST_ABS;  // dummy value, first in enum

	if (!instr_unpack(&devinfo, in_code, this)) {
		assert(false);
	}

	if (type == V3D_QPU_INSTR_TYPE_BRANCH) {
		if (!branch.ub) {
			// take over the value anyway
			branch.bdu = (v3d_qpu_branch_dest) ((in_code >> 15) & 0b111);
		}
}

/*
	// WRI DEBUG
	if (sig_magic) {
		printf("sig_magic changed to true!\n");
	}
*/
}


//////////////////////////////////////////////////////
// Calls to set the mult part of the instruction
//////////////////////////////////////////////////////

Instr &Instr::pushz() {
	flags.mpf = V3D_QPU_PF_PUSHZ;
	return *this;
}


// TODO: Where does reg go??
Instr &Instr::ldtmu(Register const &reg) {
	sig.ldtmu = true;
	sig_addr  = reg.to_waddr(); 

	return *this;
}


Instr &Instr::add(uint8_t  rf_addr1, uint8_t rf_addr2, Register const &reg3) {
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
	raddr_b       = rf_addr3; 
	alu.mul.op    = V3D_QPU_M_ADD;
	alu.mul.a     = V3D_QPU_MUX_A;
	alu.mul.b     = V3D_QPU_MUX_B;
	alu.mul.waddr = rf_addr2;
	alu.mul.magic_write = false;

	return *this;
}


Instr &Instr::sub(uint8_t  rf_addr1, uint8_t rf_addr2, Register const &reg3) {
	raddr_b       = rf_addr1; 
	alu.mul.op    = V3D_QPU_M_SUB;
	alu.mul.a     = V3D_QPU_MUX_B;
	alu.mul.b     = reg3.to_mux();
	alu.mul.waddr = rf_addr2;
	alu.mul.magic_write = false;

	return *this;
}


Instr &Instr::mov(Register const &reg,  uint8_t val) {
	alu.mul.op    = V3D_QPU_M_MOV;
	alu.mul.a     = V3D_QPU_MUX_B;
	alu.mul.b     = V3D_QPU_MUX_B;
	alu.mul.waddr = reg.to_waddr();

	return *this;
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


Instr tidx(Register const &reg) {
	Instr instr;

	instr.sig_magic  = true; 
	instr.alu.add.op = V3D_QPU_A_TIDX;
	instr.alu.add.waddr = reg.to_waddr();
	instr.alu.add.a  = V3D_QPU_MUX_R1;
	instr.alu.add.b  = V3D_QPU_MUX_R0;

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


Instr shl(Register const &reg1, Register const &reg2, uint8_t val) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.raddr_a       = reg1.to_waddr(); 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_SHL;
	instr.alu.add.a     = reg2.to_mux();
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = reg1.to_waddr();

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


Instr eidx(Register const &reg) {
	Instr instr;

	instr.alu.add.op    = V3D_QPU_A_EIDX;
	instr.alu.add.waddr = reg.to_waddr();
	instr.alu.add.a     = V3D_QPU_MUX_R2;
	instr.alu.add.b     = V3D_QPU_MUX_R0;

	return instr;
}


Instr add(Register const &reg1, Register const &reg2, Register const &reg3) {
	Instr instr;

	instr.alu.add.op    = V3D_QPU_A_ADD;
	instr.alu.add.waddr = reg1.to_waddr();
	instr.alu.add.a     = reg2.to_mux();
	instr.alu.add.b     = reg3.to_mux();

	return instr;
}


Instr add(uint8_t rf_addr1, uint8_t rf_addr2, Register const &reg3) {
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


Instr mov(uint8_t rf_addr, uint8_t val) {
	Instr instr;

	instr.sig.small_imm = true; 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_OR;
	instr.alu.add.a     = V3D_QPU_MUX_B;
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = rf_addr;
	instr.alu.add.magic_write = false;

	return instr;
}


Instr mov(Register const &reg, uint8_t rf_addr) {
	Instr instr;

	instr.raddr_a       = rf_addr; 
	instr.alu.add.op    = V3D_QPU_A_OR;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = V3D_QPU_MUX_A;
	instr.alu.add.waddr = reg.to_waddr();
	instr.alu.add.magic_write = true;

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


Instr branch(int target, int current) {
	Instr instr;

	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	instr.branch.cond = V3D_QPU_BRANCH_COND_NA0;  // TODO should be a parameter;  TODO fix in dump output
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


Instr barrierid(v3d_qpu_waddr waddr) {
	Instr instr;

	instr.alu.add.op    = V3D_QPU_A_BARRIERID;
	instr.alu.add.a     = V3D_QPU_MUX_R4;
	instr.alu.add.b     = V3D_QPU_MUX_R2;
	instr.alu.add.waddr = waddr;

/*
	instr.sig.small_imm = true; 
	instr.raddr_b       = val1; 
	instr.alu.add.magic_write = false;
*/

	return instr;
}

}  // instr
}  // v3d
}  // QPULib
