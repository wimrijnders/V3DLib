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
Instr const nop(0x3c003186bb800000);  // This is actually 'nop nop'

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


Instr ldunifrf(uint8_t rf_address) {
	Instr instr(nop);

	instr.sig.ldunifrf = true; 
	instr.sig_addr = rf_address; 

	return instr;
}


Instr tidx(v3d_qpu_waddr reg) {
	Instr instr(nop);

	instr.sig_magic  = true; 
	instr.alu.add.op = V3D_QPU_A_TIDX;
	instr.alu.add.waddr = reg;
	instr.alu.add.a  = V3D_QPU_MUX_R1;
	instr.alu.add.b  = V3D_QPU_MUX_R0;

	return instr;
}


// TODO: where should reg2 go??
Instr shr(v3d_qpu_waddr reg1, v3d_qpu_waddr reg2, uint8_t val) {
	Instr instr(nop);

	instr.sig.small_imm = true; 
	instr.sig_magic     = true; 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_SHR;
	instr.alu.add.waddr = reg1;
	instr.alu.add.b     = V3D_QPU_MUX_B;

	return instr;
}


Instr shl(v3d_qpu_waddr reg1, uint8_t rf_addr, uint8_t val) {
	Instr instr(nop);

	instr.sig.small_imm = true; 
	instr.raddr_a       = rf_addr; 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_SHL;
	instr.alu.add.a     = V3D_QPU_MUX_A;
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.waddr = reg1;

	return instr;
}


// 'and' is a keyword
Instr band(uint8_t rf_address, v3d_qpu_mux reg, uint8_t val) {
	Instr instr(nop);

	instr.sig.small_imm = true; 
	instr.raddr_b       = val; 
	instr.alu.add.op    = V3D_QPU_A_AND;
	instr.alu.add.waddr = rf_address;
	instr.alu.add.a     = reg;
	instr.alu.add.b     = V3D_QPU_MUX_B;
	instr.alu.add.magic_write = false;

	return instr;
}


Instr eidx(v3d_qpu_waddr reg) {
	Instr instr(nop);

	instr.alu.add.op    = V3D_QPU_A_EIDX;
	instr.alu.add.waddr = reg;
	instr.alu.add.a     = V3D_QPU_MUX_R2;
	instr.alu.add.b     = V3D_QPU_MUX_R0;

	return instr;
}

}  // instr
}  // v3d
}  // QPULib
