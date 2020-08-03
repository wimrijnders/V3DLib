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

Register const r0(V3D_QPU_WADDR_R0, V3D_QPU_MUX_R0);
Register const r1(V3D_QPU_WADDR_R1, V3D_QPU_MUX_R1);

uint64_t const Instr::NOP = 0x3c003186bb800000;  // This is actually 'nop nop'

void Instr::dump() const {
	char buffer[10*1024];
	instr_dump(buffer, const_cast<Instr *>(this));

	printf("%llu:\n%s", code(), buffer);
}


uint64_t Instr::code() const {
	init_ver();

  uint64_t repack = instr_pack(&devinfo, const_cast<Instr *>(this));
	return repack;
}


void Instr::show(uint64_t code) {
	Instr instr(code);
	instr.dump();
}


void Instr::init_ver() const {
	if (devinfo.ver != 42) {               //        <-- only this needs to be set
		devinfo.ver = 42;
	}
}


void Instr::init(uint64_t code) {
	init_ver();

	// These do not get initialized in unpack
	sig_addr = 0;

	if (!instr_unpack(&devinfo, code, this)) {
		assert(false);
	}
}


//////////////////////////////////////////////////////
// Calls to set the mult part of the instruction
//////////////////////////////////////////////////////

Instr Instr::add(uint8_t  rf_addr1, uint8_t rf_addr2, Register const &reg3) {
	raddr_b       = rf_addr1; 
	alu.mul.op    = V3D_QPU_M_ADD;
	alu.mul.a     = V3D_QPU_MUX_B;
	alu.mul.b     = reg3.to_mux();
	alu.mul.waddr = rf_addr2;
	alu.mul.magic_write = false;

	return *this;
}


Instr Instr::mov(Register const &reg,  uint8_t val) {
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
	instr.alu.add.op    = V3D_QPU_A_ADD;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = reg3.to_mux();
	instr.alu.add.waddr = rf_addr1;
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


}  // instr
}  // v3d
}  // QPULib
