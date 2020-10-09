#include "Instr.h"
#include <cstdio>
#include <cstdlib>        // abs()
#include <bits/stdc++.h>  // swap()
#include "../../Support/debug.h"
#include "dump_instr.h"
#include "Support/basics.h"

namespace {

struct v3d_device_info devinfo;  // NOTE: uninitialized struct, field 'ver' must be set! For asm/disasm OK

bool is_power_of_2(int x) {
    return x > 0 && !(x & (x - 1));
}


/**
 * Source: https://thispointer.com/find-and-replace-all-occurrences-of-a-sub-string-in-c/
 */
void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr) {
    // Get the first occurrence
    size_t pos = data.find(toSearch);
    // Repeat till end is reached
    while( pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos =data.find(toSearch, pos + replaceStr.size());
    }
}


}


namespace QPULib {
namespace v3d {
namespace instr {

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


std::string Instr::mnemonic() const {
	std::string ret = instr_mnemonic(this);

	auto indent = [] (int size) -> std::string {
		const int TAB_POS = 42;  // tab position for signal in mnemonic
		size++;                  // Fudging to get position right
		std::string ret;

		while (size < TAB_POS) {
			ret << " ";
			size++;
		}

		return ret;
	};


	// Output rotate signal (not done in MESA)
	if (sig.rotate) {

		// Only two possibilities here: r5 or small imm (sig for small imm not set!)
		if (alu.mul.b == V3D_QPU_MUX_R5) {
			ret << ", r5";
		} else if (alu.mul.b == V3D_QPU_MUX_B) {
			ret << ", " << raddr_b;
		} else {
			assertq(false, "mnemonic(): unexpected mux value for mul b for rotate", true);
		}

		ret << indent(ret.size()) << "; rot";
	}

	return ret;
}


/**
 * Assign header comment to current instance
 *
 * For display purposes only, when generating a dump of the opcodes.
 */
Instr &Instr::header(std::string const &msg) {
	assertq(m_header.empty(), "Header comment already has a value when setting it", true);
	m_header = msg;
	findAndReplaceAll(m_header, "\n", "\n# ");
	return *this;
}


/**
 * Assign comment to current instance
 *
 * For display purposes only, when generating a dump of the opcodes.
 */
Instr &Instr::comment(std::string const &msg) {
	assertq(m_comment.empty(), "Comment already has a value when setting it", true);
	m_comment = msg;
	findAndReplaceAll(m_comment, "\n", "\n# ");
	return *this;
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


std::string Instr::mnemonic(uint64_t in_code) {
	Instr instr(in_code);
	return instr.mnemonic();
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
		warning("Instr:init: call to instr_unpack failed.");
		return;
	}

	if (type == V3D_QPU_INSTR_TYPE_BRANCH) {
		if (!branch.ub) {
			// take over the value anyway
			branch.bdu = (v3d_qpu_branch_dest) ((in_code >> 15) & 0b111);
		}
	}

}

namespace {

std::string binaryValue(uint64_t num) {
	const int size = sizeof(num)*8;
  std::string result; 

	for (int i = size -1; i >=0; i--) {
		bool val = ((num >> i) & 1) == 1;

		if (val) {
			result += '1';
		} else {
			result += '0';
		}

		if (i % 10 == 0) {
			result += '.';
		}
	}

	return result;
}

}  // anon namespace


//#include "broadcom/qpu/qpu_instr.h"  // V2D_QPU_BRANCH_COND_ALWAYS

// from mesa/src/broadcom/qpu/qpu_pack.c
#define QPU_MASK(high, low) ((((uint64_t)1<<((high)-(low)+1))-1)<<(low))
#define QPU_GET_FIELD(word, field) ((uint32_t)(((word)  & field ## _MASK) >> field ## _SHIFT))

#define VC5_QPU_BRANCH_MSFIGN_SHIFT         21
#define VC5_QPU_BRANCH_MSFIGN_MASK          QPU_MASK(22, 21)

#define VC5_QPU_BRANCH_COND_SHIFT           32
#define VC5_QPU_BRANCH_COND_MASK            QPU_MASK(34, 32)

// End from mesa/src/broadcom/qpu/qpu_pack.c



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


	// Hypothesis: non-usage of bdu depends on these two fields

	uint32_t cond   = QPU_GET_FIELD(code1, VC5_QPU_BRANCH_COND);
	uint32_t msfign = QPU_GET_FIELD(code1, VC5_QPU_BRANCH_MSFIGN);

        if (cond == 0) {
                //instr->branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
        } else if (V3D_QPU_BRANCH_COND_A0 + (cond - 2) <= V3D_QPU_BRANCH_COND_ALLNA)
                cond = V3D_QPU_BRANCH_COND_A0 + (cond - 2);

	printf("cond: %u, msfign: %u\n", cond, msfign);

	if (cond == 2 && msfign == V3D_QPU_MSFIGN_NONE) {
		// Zap out the bdu field and compare again
		uint64_t bdu_mask = ((uint64_t) 0b111) << 15;
		code1 = code1 & ~bdu_mask;
		code2 = code1 & ~bdu_mask;

		return code1 == code2;
	}

#ifdef DEBUG
	printf("compare_codes diff: %s\n", binaryValue(code1 ^ code2).c_str());
	printf("code1: %s\n", mnemonic(code1).c_str());
	show(code1);
	printf("code2: %s\n", mnemonic(code2).c_str());
	show(code2);
	

	breakpoint
#endif  // DEBUG

	//if (is_bu_set(code1) || is_bu_set(code2)) {
	//	return false;  // bdu may be used
	//}

	return false;
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
Instr &Instr::norn()  { set_uf(V3D_QPU_UF_NORN);  return *this; }
Instr &Instr::nornn() { set_uf(V3D_QPU_UF_NORNN); return *this; }
Instr &Instr::andn()  { set_uf(V3D_QPU_UF_ANDN);  return *this; }
Instr &Instr::andz()  { set_uf(V3D_QPU_UF_ANDZ);  return *this; }
Instr &Instr::andc()  { set_uf(V3D_QPU_UF_ANDC);  return *this; }
Instr &Instr::andnc() { set_uf(V3D_QPU_UF_ANDNC); return *this; }
Instr &Instr::andnn() { set_uf(V3D_QPU_UF_ANDNN); return *this; }

Instr &Instr::ifnb()  { set_c(V3D_QPU_COND_IFNB); return *this; }
Instr &Instr::ifb()   { set_c(V3D_QPU_COND_IFB);  return *this; }
Instr &Instr::ifna()  { set_c(V3D_QPU_COND_IFNA); return *this; }
Instr &Instr::ifa()   { set_c(V3D_QPU_COND_IFA);  return *this; }


Instr &Instr::ldtmu(Register const &reg) {
	sig.ldtmu = true;
	sig_addr  = reg.to_waddr(); 
	sig_magic = true;

	return *this;
}


Instr &Instr::thrsw()   { sig.thrsw   = true; return *this; }
Instr &Instr::ldvary()  { sig.ldvary  = true; return *this; }
Instr &Instr::ldunif()  { sig.ldunif  = true; return *this; }
Instr &Instr::ldunifa() { sig.ldunifa = true; return *this; }
Instr &Instr::ldvpm()   { sig.ldvpm   = true; return *this; }

Instr &Instr::ldunifarf(RFAddress const &addr) {
	sig.ldunifarf = true;
	sig_addr = addr.to_waddr();
	sig_magic = false;
	return *this;
}

//
// Conditions  branch instructions
//
Instr &Instr::set_branch_condition(v3d_qpu_branch_cond cond) {
	assert(type == V3D_QPU_INSTR_TYPE_BRANCH);  // Branch instruction-specific
	branch.cond = cond;
	return *this;
}


Instr &Instr::a0()     { return set_branch_condition(V3D_QPU_BRANCH_COND_A0); }
Instr &Instr::na0()    { return set_branch_condition(V3D_QPU_BRANCH_COND_NA0); }
Instr &Instr::alla()   { return set_branch_condition(V3D_QPU_BRANCH_COND_ALLA); }
Instr &Instr::allna()  { return set_branch_condition(V3D_QPU_BRANCH_COND_ALLNA); }
Instr &Instr::anya()   { return set_branch_condition(V3D_QPU_BRANCH_COND_ANYA); }
Instr &Instr::anyaq()  { branch.msfign =  V3D_QPU_MSFIGN_Q; return anya(); }
Instr &Instr::anyap()  { branch.msfign =  V3D_QPU_MSFIGN_P; return anya(); }
Instr &Instr::anyna()  { return set_branch_condition(V3D_QPU_BRANCH_COND_ANYNA); }
Instr &Instr::anynaq() { branch.msfign =  V3D_QPU_MSFIGN_Q; return anyna(); }
Instr &Instr::anynap() { branch.msfign =  V3D_QPU_MSFIGN_P; return anyna(); }

// End Conditions  branch instructions


Instr &Instr::add(Location const &loc1, Location const &loc2, Location const &loc3) {
	m_doing_add = false;
	alu_mul_set(loc1, loc2, loc3); 
	alu.mul.op    = V3D_QPU_M_ADD;
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


Instr &Instr::nop() {
	m_doing_add = false;
	// With normal usage, the mul-part is already nop
	return *this;
}


Instr &Instr::mov(Location const &dst,  SmallImm const &imm) {
	m_doing_add = false;
	alu_mul_set_dst(dst);
	alu_mul_set_imm_a(imm);

	alu.mul.op    = V3D_QPU_M_MOV;
	alu.mul.b     = V3D_QPU_MUX_B;   // Apparently needs to be set also

	return *this;
}


Instr &Instr::fmov(Location const &dst,  SmallImm const &imma) {
	m_doing_add = false;
	alu_mul_set_dst(dst);
	alu_mul_set_imm_a(imma);

	alu.mul.op    = V3D_QPU_M_FMOV;
	alu.mul.b     = V3D_QPU_MUX_B;   // Apparently needs to be set also

	return *this;
}


/**
 * Can't consolidate this yet, required for special register vpm
 */
Instr &Instr::mov(uint8_t rf_addr, Register const &reg) {
	m_doing_add = false;

	alu.mul.op    = V3D_QPU_M_MOV;
	alu.mul.a     = reg.to_mux();
	alu.mul.b     = V3D_QPU_MUX_B;
	alu.mul.waddr = rf_addr;

	return *this;
}


Instr &Instr::mov(Location const &loc1, Location const &loc2) {
	m_doing_add = false;
	alu_mul_set(loc1, loc2, loc2); 

	alu.mul.op    = V3D_QPU_M_MOV;
	return *this;
}


Instr &Instr::fmul(Location const &loc1, Location const &loc2, Location const &loc3) {
	m_doing_add = false;
	alu_mul_set(loc1, loc2, loc3);

	alu.mul.op    = V3D_QPU_M_FMUL;

	return *this;
}


Instr &Instr::fmul(Location const &loc1, SmallImm imm2, Location const &loc3) {
	m_doing_add = false;
	alu_mul_set_dst(loc1);
	alu_mul_set_imm_a(imm2);

	// NOTE: raddr_a set for loc3 and b-fields used in mul
	raddr_a = loc3.to_waddr();
	alu.mul.b     = V3D_QPU_MUX_A;
	alu.mul.b_unpack = loc3.input_unpack();

	alu.mul.op    = V3D_QPU_M_FMUL;
	return *this;
}


Instr &Instr::fmul(Location const &loc1, Location const &loc2, SmallImm const &imm3) {
	alu_mul_set(loc1, loc2,  imm3);
	alu.mul.op    = V3D_QPU_M_FMUL;

	return *this;
}


Instr &Instr::smul24(Location const &dst, Location const &loca, Location const &locb) {
	m_doing_add = false;
	alu_mul_set(dst, loca, locb);

	alu.mul.op    = V3D_QPU_M_SMUL24;
	return *this;
}


/**
 * NOTE: Added this one myself, not sure if correct
 * TODO verify correctness
 */
Instr &Instr::smul24(Location const &dst, SmallImm const &imma, Location const &locb) {
	m_doing_add = false;
	alu_mul_set(dst, imma, locb);

	alu.mul.op    = V3D_QPU_M_SMUL24;
	return *this;
}


/**
 * TODO verify correctness
 */
Instr &Instr::smul24(Location const &dst, Location const &loca, SmallImm const &immb) {
	m_doing_add = false;
	alu_mul_set(dst, loca, immb);

	alu.mul.op    = V3D_QPU_M_SMUL24;
	return *this;
}


Instr &Instr::vfmul(Location const &rf_addr1, Register const &reg2, Register const &reg3) {
	m_doing_add = false;
	alu_mul_set(rf_addr1, reg2, reg3);

	alu.mul.op = V3D_QPU_M_VFMUL;
	return *this;
}


void Instr::alu_add_set_dst(Location const &loc1) {
	if (loc1.is_rf()) {
		alu.add.magic_write = false;
	} else {
		alu.add.magic_write = true;
	}

	alu.add.waddr = loc1.to_waddr();
	alu.add.output_pack = loc1.output_pack();
}


void Instr::alu_add_set_reg_a(Location const &loc2) {
	if (loc2.is_rf()) {
		raddr_a = loc2.to_waddr();
		alu.add.a     = V3D_QPU_MUX_A;
	} else {
		alu.add.a     = loc2.to_mux();
	}

	alu.add.a_unpack = loc2.input_unpack();
}


void Instr::alu_add_set_reg_b(Location const &loc3) {
	if (loc3.is_rf()) {
		// Prefer raddr_b - strictly speaking not required
	  // TODO: This might be why instructions (currently 1) fail in disasm kernel, examine
		if (!sig.small_imm) {
			// This should always be OK, add.a will never use raddr_b at this point
			raddr_b          = loc3.to_waddr(); 
			alu.add.b        = V3D_QPU_MUX_B;
		} else {
			// raddr_b contains a small imm, do raddr_a instead
			assert(alu.add.a == V3D_QPU_MUX_B);

			raddr_a          = loc3.to_waddr(); 
			alu.add.b        = V3D_QPU_MUX_A;
		}
	} else {
		alu.add.b        = loc3.to_mux();
	}

	alu.add.b_unpack = loc3.input_unpack();
}


/**
 * Set the immediate value for an operation
 *
 * The immediate value is always set in raddr_b.
 * Multiple immediate operands are allowed in an instruction only if they are the same value
 */
void Instr::alu_set_imm(SmallImm const &imm) {
	if (sig.small_imm == true) {
		if (raddr_b != imm.to_raddr()) {
			fatal("Multiple immediate values in an operation only allowed if they are the same value");
		}
	} else {
		// All is well
		sig.small_imm = true; 
		raddr_b       = imm.to_raddr(); 
	}
}


void Instr::alu_add_set_imm_a(SmallImm const &imm) {
	alu_set_imm(imm);
	alu.add.a        = V3D_QPU_MUX_B;
	alu.add.a_unpack = imm.input_unpack();
}


void Instr::alu_add_set_imm_b(SmallImm const &imm) {
	alu_set_imm(imm);
	alu.add.b        = V3D_QPU_MUX_B;
	alu.add.b_unpack = imm.input_unpack();
}


/**
 * Copied from alu_add_set_imm_a(), not sure about this
 * TODO verify in some way
 */
void Instr::alu_mul_set_imm_a(SmallImm const &imm) {
	alu_set_imm(imm);
	alu.mul.a        = V3D_QPU_MUX_B;
	alu.mul.a_unpack = imm.input_unpack();
}


void Instr::alu_mul_set_imm_b(SmallImm const &imm) {
	alu_set_imm(imm);
	alu.mul.b     = V3D_QPU_MUX_B;
	alu.mul.b_unpack = imm.input_unpack();
}


void Instr::alu_add_set(Location const &dst, Location const &srca, Location const &srcb) {
	alu_add_set_dst(dst);
	alu_add_set_reg_a(srca);
	alu_add_set_reg_b(srcb);
}


void Instr::alu_add_set(Location const &dst, SmallImm const &imma, Location const &srcb) {
	alu_add_set_dst(dst);
	alu_add_set_imm_a(imma);
	alu_add_set_reg_b(srcb);
}


void Instr::alu_add_set(Location const &dst, Location const &srca, SmallImm const &immb) {
	alu_add_set_dst(dst);
	alu_add_set_reg_a(srca);
	alu_add_set_imm_b(immb);
}


void Instr::alu_add_set(Location const &dst, SmallImm const &imma, SmallImm const &immb) {
	alu_add_set_dst(dst);
	alu_add_set_imm_a(imma);
	alu_add_set_imm_b(immb);
}


void Instr::alu_mul_set_dst(Location const &loc1) {
	if (loc1.is_rf()) {
		alu.mul.magic_write = false;
	} else {
		alu.mul.magic_write = true;
	}

	alu.mul.waddr = loc1.to_waddr();
	alu.mul.output_pack = loc1.output_pack();
}


bool Instr::raddr_a_is_safe(Location const &loc) const {
	// Is raddr_a in use by add alu?
	bool raddr_a_in_use = (alu.add.a == V3D_QPU_MUX_A) || (alu.add.b == V3D_QPU_MUX_A);

	// Is it by chance the same value?
	bool raddr_a_same = (raddr_a == loc.to_waddr());

	return (!raddr_a_in_use || raddr_a_same);
}


void Instr::alu_mul_set_reg_a(Location const &loc2) {
	if (!loc2.is_rf()) {
		// src is a register
		alu.mul.a     = loc2.to_mux();
	} else {
		// src is a register file index

		if (raddr_a_is_safe(loc2)) {
			raddr_a   = loc2.to_waddr();  // This could overwrite with the same value
			alu.mul.a = V3D_QPU_MUX_A;
		} else {
			// Use raddr_b instead
			assertq(!(alu.add.a == V3D_QPU_MUX_B) || (alu.add.b == V3D_QPU_MUX_B),
			  "alu_mul_set_reg_a: both raddr a and b in use by add alu");

			raddr_b    = loc2.to_waddr();  // This could 
			alu.mul.a  = V3D_QPU_MUX_B;
		}
	}

	alu.mul.a_unpack = loc2.input_unpack();
}


void Instr::alu_mul_set_reg_b(Location const &loc3) {
	if (!loc3.is_rf()) {
		alu.mul.b        = loc3.to_mux();
	} else {
		if (alu.mul.a == V3D_QPU_MUX_B) {
			//assertq((raddr_b == loc3.to_waddr()), "alu_mul_set_reg_b: raddr b in use by mul alu a with different value");

			if (raddr_a_is_safe(loc3)) {
				raddr_a          = loc3.to_waddr(); 
				alu.mul.b        = V3D_QPU_MUX_A;
			} else {
				debug_break("alu_mul_set_reg_b(): raddr_a in use by add alu and raddr_b in use for immediate");	
			}
		} else {
			raddr_b          = loc3.to_waddr(); 
			alu.mul.b        = V3D_QPU_MUX_B;
		}
	}

	alu.mul.b_unpack = loc3.input_unpack();
}


void Instr::alu_mul_set(Location const &loc1, Location const &loc2, Location const &loc3) {
	alu_mul_set_dst(loc1);
	alu_mul_set_reg_a(loc2);
	alu_mul_set_reg_b(loc3);
}


void Instr::alu_mul_set(Location const &loc1, Location const &loc2, SmallImm const &imm3) {
	alu_mul_set_dst(loc1);
	alu_mul_set_reg_a(loc2);
	alu_mul_set_imm_b(imm3);
}


void Instr::alu_mul_set(Location const &dst, SmallImm const &imma, Location const &locb) {
	alu_mul_set_dst(dst);
	alu_mul_set_imm_a(imma);
	alu_mul_set_reg_b(locb);
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

	instr.sig.ldunifrf = true; 
	instr.sig_addr = rf_address; 

	return instr;
}


Instr shr(Location const &dst, Location const &srca, SmallImm const &immb) {
	Instr instr;
	instr.alu_add_set(dst, srca, immb);

	instr.alu.add.op    = V3D_QPU_A_SHR;
	instr.sig_magic     = true;    // TODO: need this? Also for shl?

	return instr;
}


Instr shl(Location const &dst, Location const &srca, SmallImm const &immb) {
	Instr instr;
	instr.alu_add_set(dst, srca, immb);

	instr.alu.add.op    = V3D_QPU_A_SHL;
	//?? instr.sig_magic     = true;  // Set in shr, need it here?

	return instr;
}


Instr shl(Location const &dst, Location const &srca, Location const &srcb) {
	Instr instr;
	instr.alu_add_set(dst, srca, srcb);

	instr.alu.add.op    = V3D_QPU_A_SHL;
	//?? instr.sig_magic     = true;  // Set in shr, need it here?

	return instr;
}


Instr shl(Location const &dst, SmallImm const &imma, SmallImm const &immb) {
	assertq(imma == immb, "Operands a and b can only be both immediates if they are the exact same value");

	Instr instr;
	instr.alu_add_set_dst(dst);
	instr.alu_add_set_imm_a(imma);
	instr.alu_add_set_imm_b(immb);

	instr.alu.add.op    = V3D_QPU_A_SHL;
	//?? instr.sig_magic     = true;  // Set in shr, need it here?

	return instr;
}


Instr asr(Location const &loc1, Location const &loc2, SmallImm const &imm3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2,  imm3);

	instr.alu.add.op    = V3D_QPU_A_ASR;
	return instr;
}


Instr itof(Location const &dst, Location const &srca, SmallImm const &immb) {
	Instr instr;
	instr.alu_add_set(dst, srca,  immb);  // TODO: why would a small imm be required here??
	instr.alu.add.op = V3D_QPU_A_ITOF;
	return instr;
}


Instr ftoi(Location const &dst, Location const &srca, SmallImm const &immb) {
	Instr instr;
	instr.alu_add_set(dst, srca,  immb);  // TODO: why would a small imm be required here??
	instr.alu.add.op = V3D_QPU_A_FTOIN;   // Also possible here: V3D_QPU_A_FTOIZ
	                                      // TODO: Examine how to handle this, which is best
	return instr;
}


/**
 * 'and' is a keyword, hence prefix 'b'
 */
Instr band(Location const &dst, Location const &srca, SmallImm const &immb) {
	Instr instr;
	instr.alu_add_set(dst, srca, immb);

	instr.alu.add.op    = V3D_QPU_A_AND;
	return instr;
}


/**
 * Returns index of current vector item on a given QPU.
 * This will be something in the range [0..15]
 */
Instr eidx(Location const &reg) {
	Instr instr;
	instr.alu_add_set_dst(reg);

	instr.alu.add.op    = V3D_QPU_A_EIDX;
	instr.alu.add.a     = V3D_QPU_MUX_R2;
	instr.alu.add.b     = V3D_QPU_MUX_R0;

	return instr;
}


Instr tidx(Location const &reg) {
	Instr instr;
	instr.alu_add_set_dst(reg);

	instr.sig_magic  = true;  // TODO is this really needed? Not present in eidx
	instr.alu.add.op = V3D_QPU_A_TIDX;
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


Instr add(Location const &loc1, Location const &loc2, SmallImm const &imm3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, imm3);

	instr.alu.add.op    = V3D_QPU_A_ADD;
	return instr;
}


Instr sub(Location const &loc1, Location const &loc2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, loc3);
	instr.alu.add.op = V3D_QPU_A_SUB;
	return instr;
}


Instr sub(Location const &loc1, Location const &loc2, SmallImm const &imm3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, imm3);
	instr.alu.add.op = V3D_QPU_A_SUB;
	return instr;
}


Instr sub(Location const &dst, SmallImm const &imma, Location const &srcb) {
	Instr instr;
	instr.alu_add_set(dst, imma, srcb);
	instr.alu.add.op = V3D_QPU_A_SUB;
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

	// hypothesis: magic_write true selects register, false selects address in register file 
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

	instr.alu.add.op    = V3D_QPU_A_OR;
	instr.alu.add.b     = V3D_QPU_MUX_A;
	instr.alu.add.waddr = reg.to_waddr();
	instr.alu.add.magic_write = true;

	return instr;
}


Instr mov(Location const &loc1, Location const &loc2) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, loc2);

	instr.alu.add.op    = V3D_QPU_A_OR;

	return instr;
}


// or is reserved keyword
Instr bor(Location const &dst, Location const &srca, Location const &srcb) {
	Instr instr;
	instr.alu_add_set(dst, srca, srcb);

	instr.alu.add.op    = V3D_QPU_A_OR;
	return instr;
}


Instr bor(Location const &dst, SmallImm const &imma, SmallImm const &immb) {
	Instr instr;
	instr.alu_add_set(dst, imma, immb);

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


///////////////////////////////////////////////////////////////////////////////
// Label support
//////////////////////////////////////////////////////////////////////////////

void Instr::label(int val) {
	assert(val >= 0);
	m_label = val;
}


int Instr::branch_label() const {
	assert(is_branch_label());
	return m_label;
}


bool Instr::is_branch_label() const {
	return type == V3D_QPU_INSTR_TYPE_BRANCH && m_label >= 0;
}


void Instr::label_to_target(int offset) {
	assert(!m_is_label);
	assert(0 <= branch_label());  // only do this to a branch instruction with a label
	assert(branch.offset == 0);   // Shouldn't have been set already

	// branch needs 4 delay slots before executing, hence the 4
	// This means that 3 more instructions will execute after the loop before jumping
	branch.offset = (unsigned) 8*(offset - 4);

	m_label = -1;
}

///////////////////////////////////////////////////////////////////////////////
// End Label support
///////////////////////////////////////////////////////////////////////////////


/**
 * Jump relative
 *
 * This creates an unconditionial jump.
 * Add conditions with the associated methods, eg. `na0()`
 *
 * TODO: can we get rid of this in favor of  the override?
 */
Instr branch(int target, int current) {
	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
	instr.branch.ub = false;

	instr.branch.bdi = V3D_QPU_BRANCH_DEST_REL;  // branch dest
	instr.branch.bdu = V3D_QPU_BRANCH_DEST_REL;  // not used when branch.ub == false, just set a value

	instr.branch.msfign = V3D_QPU_MSFIGN_NONE;
	instr.branch.raddr_a = 0;

	// branch needs 4 delay slots before executing, hence the 4
	// This means that 3 more instructions will execute after the loop before jumping
	instr.branch.offset = (unsigned) 8*(target - (current + 4));

	return instr;
}


/**
 * Jump absolute
 *
 * This creates an unconditionial jump.
 * Add conditions with the associated methods, eg. `na0()`
 */
Instr branch(int target, bool relative) {
	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
	instr.branch.ub = false;

	// branch needs 4 delay slots before executing
	// This means that 3 more instructions will execute after the loop before jumping
	// The offset value must be compensated for this

	if (relative) {
		instr.branch.bdi = V3D_QPU_BRANCH_DEST_REL;
		instr.branch.bdu = V3D_QPU_BRANCH_DEST_REL;  // not used when branch.ub == false, just set a value

		// Asumption: relative jump need not be compensated
		instr.branch.offset = (unsigned) 8*target;  // TODO check if ok
	} else {
		breakpoint
		instr.branch.bdi = V3D_QPU_BRANCH_DEST_ABS;
		instr.branch.bdu = V3D_QPU_BRANCH_DEST_ABS;  // not used when branch.ub == false, just set a value
		instr.branch.offset = (unsigned) 8*(target - 4);  // TODO check if ok
	}

	instr.branch.msfign = V3D_QPU_MSFIGN_NONE;
	instr.branch.raddr_a = 0;


	return instr;
}


/**
 * Actually called just 'b' in the mnemonics
 */
Instr bb(Location const &loc1) {
	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	// Values instr.sig   not important
	// Values instr.flags not important

	instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
	instr.branch.ub =  false;

	instr.branch.bdi =  V3D_QPU_BRANCH_DEST_REGFILE;
	instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
	instr.branch.raddr_a = loc1.to_waddr();
	instr.branch.offset = 0;

	return instr;
}


Instr bb(BranchDest const &loc1) {
	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	// Values instr.sig   not important
	// Values instr.flags not important

	instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
	instr.branch.ub =  false;

	instr.branch.bdi =  V3D_QPU_BRANCH_DEST_LINK_REG;
	instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
	instr.branch.raddr_a = loc1.to_waddr();
	instr.branch.offset = 0;

	return instr;
}


Instr bb(uint32_t addr) {
	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	// Values instr.sig   not important
	// Values instr.flags not important

	instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
	instr.branch.ub =  false;

	instr.branch.bdi =  V3D_QPU_BRANCH_DEST_ABS;
	instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;

	//instr.branch.raddr_a = loc1.to_waddr();
	instr.branch.offset = addr;

	return instr;
}


Instr bu(uint32_t addr, Location const &loc2) {
	//printf("called bu(uint32_t addr, Location const &loc2)\n");

	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	// Values instr.sig   not important
	// Values instr.flags not important

	instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
	instr.branch.ub =  true;

	instr.branch.bdi =  V3D_QPU_BRANCH_DEST_ABS;
	//instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
	instr.branch.bdu =  V3D_QPU_BRANCH_DEST_REGFILE;

	instr.branch.raddr_a = loc2.to_waddr();
	instr.branch.offset = addr;

	return instr;
}


/**
 * NOTE: loc2 not used?
 */
Instr bu(BranchDest const &loc1, Location const &loc2) {
	//printf("called Instr bu(BranchDest const &loc1, Location const &loc2)\n");

	Instr instr;
	instr.type = V3D_QPU_INSTR_TYPE_BRANCH;

	// Values instr.sig   not important
	// Values instr.flags not important

	instr.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
	instr.branch.ub =  true;

	instr.branch.bdi =  V3D_QPU_BRANCH_DEST_LINK_REG;

	// Hackish!
	auto bd_p = dynamic_cast<Register const *> (&loc2);
	if (bd_p == nullptr) {
		// The regular path
		instr.branch.bdu =  V3D_QPU_BRANCH_DEST_REL;
	} else {
		if (bd_p->name() == "r_unif") {
			instr.branch.bdu =  V3D_QPU_BRANCH_DEST_REL;
		} else  if (bd_p->name() == "a_unif") {
				instr.branch.bdu =  V3D_QPU_BRANCH_DEST_ABS;
		} else {
			assert(false);  // Not expecting anything else
		}
	}
	
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


Instr fsub(Location const &loc1, Location const &loc2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, loc3);

	instr.alu.add.op    = V3D_QPU_A_FSUB;

	return instr;
}


Instr fsub(Location const &loc1, SmallImm const &imm2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set(loc1, imm2, loc3);

	instr.alu.add.op    = V3D_QPU_A_FSUB;
	return instr;
}


Instr vfpack(Location const &loc1, Location const &loc2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set_dst(loc1);

	instr.raddr_a       = loc2.to_waddr();
	instr.alu.add.op    = V3D_QPU_A_VFPACK;
	instr.alu.add.a     = loc2.to_mux();
	instr.alu.add.b     = loc3.to_mux();
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

	instr.alu.add.waddr = loc1.to_waddr();
	instr.alu.add.magic_write = false;

	instr.alu.add.a     = V3D_QPU_MUX_A;

	instr.raddr_b       = loc1.to_waddr();
	instr.alu.add.b     = V3D_QPU_MUX_R0;

	instr.alu.add.op    = V3D_QPU_A_VFLB;

	return instr;
}


Instr vfmin(Location const &loc1, SmallImm imm2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set_dst(loc1);
	instr.alu_add_set_imm_a(imm2);
	instr.alu_add_set_reg_b(loc3);

	instr.alu.add.op    = V3D_QPU_A_VFMIN;

	return instr;
}


Instr vfmin(Location const &loc1, Location const &loc2, Location const &loc3) {
	Instr instr;
	instr.alu_add_set(loc1, loc2, loc3);
	instr.alu.add.op    = V3D_QPU_A_VFMIN;

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

	return instr;
}


/**
 * Perform full rotate with offset in r5 using add ALU.
 *
 * - dest is r1
 * - reg a is r0
 * - reg b if used is r5, otherwise smallimm with specific range passed (see override below)
 * - uses mov as opcode
 *
 * Since dest, src are fixed, these are not passed in.
 * If this conflicts with syntax of any other assemblers, change this
 * (it already conflicts with python6 assembler).
 *
 * TODO: both add and mul can do rotate in v3d, fix.
 *
 * ============================================================================
 * NOTES
 * -----
 * 
 * * Message from the `py-videocore6` maintainer:
 *
 *    > Yes, rotate only works on mul ALU as in VC4 QPU.
 *
 * * Rotate signal is not outputted in broadcom menmonic dump.
 *   This is most likely because rotate is of no use to MESA.
 *
 * * From python6 project(test_signals.py):
 *
 *   - nop required before rotate (but lines 82, 147 only done once before loop)
 *   - Smallimm offset in range -15,16 inclusive; 'i == offset' in points below
 *
 *   1. rot signal with rN source performs as a full rotate
 *     - nop().add(r1, r0, r0, sig = rot(i))  # Also with r5=offset, rot signal still used!
 *   2. rotate alias
 *     - rotate(r1, r0, i)       # add alias, 'i % 1 == 0' ??? Always true
 *     - nop().rotate(r1, r0, i) # mul alias
 *     - rotate(r1, r0, r5)       # add alias
 *     - nop().rotate(r1, r0, r5) # mul alias
 *   3. rot signal with rfN source performs as a quad rotate
 *     - nop().add(r1, rf32, rf32, sig = rot(i))
 *     - nop().add(r1, rf32, rf32, sig = rot(r5))
 *   4. quad_rotate alias
 *     - quad_rotate(r1, rf32, i)       # add alias
 *     - nop().quad_rotate(r1, rf32, i) # mul alias
 *     - quad_rotate(r1, rf32, r5)       # add alias
 *     - nop().quad_rotate(r1, rf32, r5) # mul alias
 *   5. instruction with r5rep dst performs as a full broadcast
 *     - Uses rot signal with special condition
 *     -  Skip for now
 *   6. broadcast alias
 *     - idem 5, skip for now
 *   7. instruction with r5 dst performs as a quad broadcast
 *     - idem 5, skip for now
 *
 * * Conclusions previous point:
 *
 *   Only 2. relevant for QPULib code, skip rest for now
 *
 *   - nop required before rotate (but lines 82, 147 only done once before loop)
 *   - Only mul alu can do rotate (vc4 AND v3d)
 *   - dst apparently always r1
 *   - src apparently always r0 for 'full rotate'; TODO likely not true, check
 *   - offset is either a SmallImm or in r5
 *   - Smallimm offset in range -15,16 inclusive
 *   - TODO: try to understand the newfangled quad rotate shit.
 *
 */
Instr rotate(Location const &dst, Location const &loca, Location const &locb) {
	Instr instr;
	return instr.rotate(dst, loca, locb);  // Use mul alu version
}


/**
 * Rotate.
 *
 * Rotate only works via the mul ALU.
 *
 * See notes in header comment of rotate overload above.
 */
Instr rotate(Location const &dst, Location const &loca, SmallImm const &immb) {
	Instr instr;
	return instr.rotate(dst, loca, immb);  // Use mul alu version
}


/**
 * Rotate for mul alu.
 *
 * Rotate only works via the mul ALU.
 *
 * See notes in header comment of rotate overload for add alu above.
 */
Instr &Instr::rotate(Location const &dst, Location const &loca, SmallImm const &immb) {
	//warning("rotate called, really not sure if correct.");
	assertq(dst.to_mux()  == V3D_QPU_MUX_R1, "rotate dest can only be r1");
	assertq(loca.to_mux() == V3D_QPU_MUX_R0, "rotate src a can only be r0");
	assertq(-15 <= immb.val() && immb.val() < 16, "rotate: smallimm must be in proper range");

	m_doing_add = false;

	alu_mul_set(r1, r0, immb);

	if (immb.val() != 0) {  // Don't bother rotating if there is no rotate
		sig.rotate = true;
	}
	sig.small_imm = false;      // Should *not* be set for rotate
	alu.mul.op = V3D_QPU_M_MOV;

	return *this;
}


/**
 * Rotate for mul alu.
 *
 * See notes in header comment of rotate overload for add alu above.
 */
Instr &Instr::rotate(Location const &dst, Location const &loca, Location const &locb) {
	//warning("rotate called, really not sure if correct.");
	assertq(dst.to_mux()  == V3D_QPU_MUX_R1, "rotate dest can only be r1");
	assertq(loca.to_mux() == V3D_QPU_MUX_R0, "rotate src a can only be r0");
	assertq(locb.to_mux() == V3D_QPU_MUX_R5, "rotate src b can only be r5");
	// TODO: check value r5 within range -15,15 inclusive, possible?

	m_doing_add = false;

	alu_mul_set(r1, r0, r5);
	sig.rotate = true;
	alu.mul.op = V3D_QPU_M_MOV;

	return *this;
}


/**
 * TODO: research what this is for (tmu write?), no clue right now
 */
Instr tmuwt() {
	Instr instr;
	instr.alu.add.op = V3D_QPU_A_TMUWT;

	return instr;
}


Instr min(Location const &dst, Location const &srca, Location const &srcb) {
	Instr instr;
	instr.alu_add_set(dst, srca, srcb);
	instr.alu.add.op = V3D_QPU_A_MIN;

	return instr;
}


Instr max(Location const &dst, Location const &srca, Location const &srcb) {
	Instr instr;
	instr.alu_add_set(dst, srca, srcb);
	instr.alu.add.op = V3D_QPU_A_MAX;

	return instr;
}

}  // instr
}  // v3d
}  // QPULib
