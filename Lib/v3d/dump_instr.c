#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dump_instr.h"
#include "broadcom/qpu/qpu_disasm.h"

static const struct v3d_qpu_alu_instr ALU_NOP = {
    add: {
      op: V3D_QPU_A_NOP,
      a: V3D_QPU_MUX_R0,
      b: V3D_QPU_MUX_R0,
      waddr: 6,
      magic_write: true,
      output_pack: V3D_QPU_PACK_NONE,
      a_unpack: V3D_QPU_UNPACK_NONE, 
      b_unpack: V3D_QPU_UNPACK_NONE
    },
    mul: {
      op: V3D_QPU_M_NOP,
      a: V3D_QPU_MUX_R0,
      b: V3D_QPU_MUX_R4,
      waddr: 6,
      magic_write: true,
      output_pack: V3D_QPU_PACK_NONE,
      a_unpack: V3D_QPU_UNPACK_NONE, 
      b_unpack: V3D_QPU_UNPACK_NONE
    }
};


static bool isNopAdd( const struct v3d_qpu_alu_instr *src) {
	return (
      src->add.op == ALU_NOP.add.op &&
      src->add.a == ALU_NOP.add.a &&
      src->add.b == ALU_NOP.add.b &&
      src->add.waddr == ALU_NOP.add.waddr &&
      src->add.magic_write == ALU_NOP.add.magic_write &&
      src->add.output_pack == ALU_NOP.add.output_pack &&
      src->add.a_unpack == ALU_NOP.add.a_unpack &&
      src->add.b_unpack == ALU_NOP.add.b_unpack
	);
}


static bool isNopMul( const struct v3d_qpu_alu_instr *src) {
	return (
      src->mul.op == ALU_NOP.mul.op &&
      src->mul.a == ALU_NOP.mul.a &&
      src->mul.b == ALU_NOP.mul.b &&
      src->mul.waddr == ALU_NOP.mul.waddr &&
      src->mul.magic_write == ALU_NOP.mul.magic_write &&
      src->mul.output_pack == ALU_NOP.mul.output_pack &&
      src->mul.a_unpack == ALU_NOP.mul.a_unpack &&
      src->mul.b_unpack == ALU_NOP.mul.b_unpack
	);
}

#define CASE(l)	case V3D_QPU_##l: ret = #l; break;

static const char *dump_cond(enum v3d_qpu_cond val) {
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(COND_NONE)
		CASE(COND_IFA)
		CASE(COND_IFB)
		CASE(COND_IFNA)
		CASE(COND_IFNB)
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_pf(enum v3d_qpu_pf val) {
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(PF_NONE)
		CASE(PF_PUSHZ)
		CASE(PF_PUSHN)
		CASE(PF_PUSHC)
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_uf(enum v3d_qpu_uf val) {
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(UF_NONE)
		CASE(UF_ANDZ)
		CASE(UF_ANDNZ)
		CASE(UF_NORNZ)
		CASE(UF_NORZ)
		CASE(UF_ANDN)
		CASE(UF_ANDNN)
		CASE(UF_NORNN)
		CASE(UF_NORN)
		CASE(UF_ANDC)
		CASE(UF_ANDNC)
		CASE(UF_NORNC)
		CASE(UF_NORC)
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_msfign(enum v3d_qpu_msfign val) {
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(MSFIGN_NONE)
		CASE(MSFIGN_P)
		CASE(MSFIGN_Q)
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_branch_dest(enum v3d_qpu_branch_dest val) {
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(BRANCH_DEST_ABS)
		CASE(BRANCH_DEST_REL)
		CASE(BRANCH_DEST_LINK_REG)
		CASE(BRANCH_DEST_REGFILE)
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_add_op(enum v3d_qpu_add_op val) {
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(A_FADD)
		CASE(A_FADDNF)
		CASE(A_VFPACK)
		CASE(A_ADD)
		CASE(A_SUB)
		CASE(A_FSUB)
		CASE(A_MIN)
		CASE(A_MAX)
		CASE(A_UMIN)
		CASE(A_UMAX)
		CASE(A_SHL)
		CASE(A_SHR)
		CASE(A_ASR)
		CASE(A_ROR)
		CASE(A_FMIN)
		CASE(A_FMAX)
		CASE(A_VFMIN)
		CASE(A_AND)
		CASE(A_OR)
		CASE(A_XOR)
		CASE(A_VADD)
		CASE(A_VSUB)
		CASE(A_NOT)
		CASE(A_NEG)
		CASE(A_FLAPUSH)
		CASE(A_FLBPUSH)
		CASE(A_FLPOP)
		CASE(A_RECIP)
		CASE(A_SETMSF)
		CASE(A_SETREVF)
		CASE(A_NOP)
		CASE(A_TIDX)
		CASE(A_EIDX)
		CASE(A_LR)
		CASE(A_VFLA)
		CASE(A_VFLNA)
		CASE(A_VFLB)
		CASE(A_VFLNB)
		CASE(A_FXCD)
		CASE(A_XCD)
		CASE(A_FYCD)
		CASE(A_YCD)
		CASE(A_MSF)
		CASE(A_REVF)
		CASE(A_VDWWT)
		CASE(A_IID)
		CASE(A_SAMPID)
		CASE(A_BARRIERID)
		CASE(A_TMUWT)
		CASE(A_VPMSETUP)
		CASE(A_VPMWT)
		CASE(A_LDVPMV_IN)
		CASE(A_LDVPMV_OUT)
		CASE(A_LDVPMD_IN)
		CASE(A_LDVPMD_OUT)
		CASE(A_LDVPMP)
		CASE(A_RSQRT)
		CASE(A_EXP)
		CASE(A_LOG)
		CASE(A_SIN)
		CASE(A_RSQRT2)
		CASE(A_LDVPMG_IN)
		CASE(A_LDVPMG_OUT)
		CASE(A_FCMP)
		CASE(A_VFMAX)
		CASE(A_FROUND)
		CASE(A_FTOIN)
		CASE(A_FTRUNC)
		CASE(A_FTOIZ)
		CASE(A_FFLOOR)
		CASE(A_FTOUZ)
		CASE(A_FCEIL)
		CASE(A_FTOC)
		CASE(A_FDX)
		CASE(A_FDY)
		CASE(A_STVPMV)
		CASE(A_STVPMD)
		CASE(A_STVPMP)
		CASE(A_ITOF)
		CASE(A_CLZ)
		CASE(A_UTOF)
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_mux(enum v3d_qpu_mux val) {
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(MUX_R0)
		CASE(MUX_R1)
		CASE(MUX_R2)
		CASE(MUX_R3)
		CASE(MUX_R4)
		CASE(MUX_R5)
		CASE(MUX_A)
		CASE(MUX_B)
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_output_pack(enum v3d_qpu_output_pack val) {
	static char buffer[64];
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(PACK_NONE)
		CASE(PACK_L)
		CASE(PACK_H)
		default:
			sprintf(buffer, "<<UNKNOWN>> (%u)", (unsigned) val);
			ret = buffer;
			break;
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_input_unpack(enum v3d_qpu_input_unpack val) {
//	printf("v3d_qpu_input_unpack: %u\n", val);

	static char buffer[64];
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(UNPACK_NONE)
		CASE(UNPACK_ABS)
		CASE(UNPACK_L)
		CASE(UNPACK_H)
		CASE(UNPACK_REPLICATE_32F_16)
		CASE(UNPACK_REPLICATE_L_16)
		CASE(UNPACK_REPLICATE_H_16)
		CASE(UNPACK_SWAP_16)
		default:
			sprintf(buffer, "<<UNKNOWN>> (%u)", (unsigned) val);
			ret = buffer;
			break;
	}

	assert(ret != 0);
	return ret;
}


static const char *dump_mul_op(enum v3d_qpu_mul_op val) {
	char *ret = "<<UNKNOWN>>";

	switch (val) {
		CASE(M_ADD)
		CASE(M_SUB)
		CASE(M_UMUL24)
		CASE(M_VFMUL)
		CASE(M_SMUL24)
		CASE(M_MULTOP)
		CASE(M_FMOV)
		CASE(M_MOV)
		CASE(M_NOP)
		CASE(M_FMUL)
	}

	assert(ret != 0);
	return ret;
}


#undef CASE

void instr_dump(char *buffer, struct v3d_qpu_instr *instr) {
	assert(buffer != 0);
	assert(instr != 0);

#define TF(v) (v)?"true":"false"

	bool show_alu = false;
	const char *type = NULL;
	switch(instr->type) {
		case V3D_QPU_INSTR_TYPE_ALU   :
			show_alu = true;
			type = "INSTR_TYPE_ALU";
			break;
		case V3D_QPU_INSTR_TYPE_BRANCH:
			show_alu = false;
			type = "INSTR_TYPE_BRANCH";
			break;
	}


	char buffer_sig[1024] = "\0";
	if (instr->sig.thrsw) strcat(buffer_sig, "thrsw ");
	if (instr->sig.ldunif) strcat(buffer_sig, "ldunif ");
	if (instr->sig.ldunifa) strcat(buffer_sig, "ldunifa ");
	if (instr->sig.ldunifrf) strcat(buffer_sig, "ldunifrf ");
	if (instr->sig.ldunifarf) strcat(buffer_sig, "ldunifarf ");
	if (instr->sig.ldtmu) strcat(buffer_sig, "ldtmu ");
	if (instr->sig.ldvary) strcat(buffer_sig, "ldvary ");
	if (instr->sig.ldvpm) strcat(buffer_sig, "ldvpm ");
	if (instr->sig.ldtlb) strcat(buffer_sig, "ldtlb ");
	if (instr->sig.ldtlbu) strcat(buffer_sig, "ldtlbu ");
	if (instr->sig.small_imm) strcat(buffer_sig, "small_imm ");
	if (instr->sig.ucb) strcat(buffer_sig, "ucb ");
	if (instr->sig.rotate) strcat(buffer_sig, "rotate ");
	if (instr->sig.wrtmuc) strcat(buffer_sig, "wrtmuc ");

	char buffer_union[1024] = "\0";
	char buffer_alu_add[1024] = "\0";
	char buffer_alu_mul[1024] = "\0";

	if (show_alu) {
		const char *format_alu_addmul = "\
    {\n\
      op: %s,\n\
      a: %s,\n\
      b: %s,\n\
      waddr: %u,\n\
      magic_write: %s,\n\
      output_pack: %s,\n\
      a_unpack: %s, \n\
      b_unpack: %s\n\
    }";

		const char *format_alu = "\
  alu: {\n\
    add: %s,\n\
    mul: %s\n\
  }\n\
";

		if (isNopAdd(&instr->alu)) {
			sprintf(buffer_alu_add, "NOP");
		} else {
			sprintf(buffer_alu_add, format_alu_addmul,
				dump_add_op(instr->alu.add.op),
				dump_mux(instr->alu.add.a),
				dump_mux(instr->alu.add.b),
				instr->alu.add.waddr,
				TF(instr->alu.add.magic_write),
				dump_output_pack(instr->alu.add.output_pack),
				dump_input_unpack(instr->alu.add.a_unpack),
				dump_input_unpack(instr->alu.add.b_unpack)
			);
		}

		if (isNopMul(&instr->alu)) {
			sprintf(buffer_alu_mul, "NOP");
		} else {
			sprintf(buffer_alu_mul, format_alu_addmul,
				dump_mul_op(instr->alu.mul.op),
				dump_mux(instr->alu.mul.a),
				dump_mux(instr->alu.mul.b),
				instr->alu.mul.waddr,
				TF(instr->alu.mul.magic_write),
				dump_output_pack(instr->alu.mul.output_pack),
				dump_input_unpack(instr->alu.mul.a_unpack),
				dump_input_unpack(instr->alu.mul.b_unpack)
			);
		}

		sprintf(buffer_union, format_alu, buffer_alu_add, buffer_alu_mul);

	} else {
		const char *format_branch = "\
  branch: {\n\
    cond: %u,\n\
    msfign: %s,\n\
    bdi: %s,  // branch dest\n\
    bdu: %s,\n\
    ub: %s,\n\
    raddr_a: %u,\n\
    offset: %u\n\
  }\n\
";

		sprintf(buffer_union, format_branch,
			instr->branch.cond,
			dump_msfign(instr->branch.msfign),
			dump_branch_dest(instr->branch.bdi),
			dump_branch_dest(instr->branch.bdu),
			// bdu
			TF(instr->branch.ub),
			instr->branch.raddr_a,
			instr->branch.offset
		);
	}

	const char *format = "\n{\n\
  type: %s,\n\
  sig: {%s},\n\
  sig_addr: %u,\n\
  sig_magic: %s,\n\
  raddr_a: %u ,\n\
  raddr_b: %u,\n\
  flags: {ac: %s, mc: %s, apf: %s, mpf: %s, auf: %s, muf: %s},\n\
%s\
}\n";


	sprintf(buffer, format,
		type,
		buffer_sig,
		instr->sig_addr,
		TF(instr->sig_magic),
		instr->raddr_a,
		instr->raddr_b,
		// flags
		dump_cond(instr->flags.ac),
		dump_cond(instr->flags.mc),
		dump_pf(instr->flags.apf),
		dump_pf(instr->flags.mpf),
		dump_uf(instr->flags.auf),
		dump_uf(instr->flags.muf),
		buffer_union
	);

#undef TF
}


///////////////////////////////////////////////////////////////////////////////
// Wrappers for the actual calls we want to make.
// 
// These are here to beat the external linkage issues I've been having.
///////////////////////////////////////////////////////////////////////////////

bool instr_unpack(struct v3d_device_info const *devinfo, uint64_t packed_instr, struct v3d_qpu_instr *instr) {
	return v3d_qpu_instr_unpack(devinfo, packed_instr, instr);
}


uint64_t instr_pack(struct v3d_device_info const *devinfo, struct v3d_qpu_instr const *instr) {
	uint64_t packed_instr;
	v3d_qpu_instr_pack(devinfo, instr, &packed_instr);
	return packed_instr;
}


void instr_dump_mnemonic(const struct v3d_qpu_instr *instr) {
	struct v3d_device_info devinfo;
	devinfo.ver = 42;
	v3d_qpu_dump(&devinfo, instr);
}

bool small_imm_pack(uint32_t value, uint32_t *packed_small_immediate) {
	struct v3d_device_info devinfo;
	devinfo.ver = 42;

	return v3d_qpu_small_imm_pack(&devinfo, value, packed_small_immediate);
}
