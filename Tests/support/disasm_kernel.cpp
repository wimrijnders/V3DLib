#include "disasm_kernel.h"
#include "../../Lib/Support/basics.h"
#include "v3d/instr/Instr.h"
#include "qpu_disasm.h"

namespace {

std::vector<uint64_t> bytecode;

/**
 * Check if there's nothing special with this particular code
 */
void test_unpack_pack(uint64_t in_code) {
	using namespace V3DLib::v3d::instr;

	Instr instr(in_code);  // will assert on unpack error
	assert(in_code == instr.code());
}

}  // anon namespace


std::vector<uint64_t> &qpu_disasm_bytecode() {
	if (bytecode.size() == 0) {
		for (int i = 0; i < tests_size; ++i) {
			bytecode.push_back(tests[i].inst);
		} 
	}

	return bytecode;
}


/**
 * DON'T execute this kernel on the QPU's!
 * It is just a sequence of instructions from a test
 *
 * Instructions which can't be translated, get replaced by `nop`.
 * The calling unit test must take this into account.
 */
std::vector<uint64_t> qpu_disasm_kernel() {
	using namespace V3DLib::v3d::instr;

	std::vector<Instr> ret;

	ret
		<< nop().ldvary()
		<< fadd(r1, r1, r5).thrsw()
		<< vpmsetup(r5).ldunif()
		<< nop().ldunifa()  // NB for version 33 this is `nop().ldvpm()`
		<< bor(rf(0), r3, r3).mov(vpm, r3)

		// ver 42, error in instr_unpack():     WARNING: Instr:init: call to instr_unpack failed.
		// { 33, 0x57403006bbb80000ull, "nop                  ; fmul  r0, rf0, r5 ; ldvpm; ldunif" },
		<< nop()

		<< ffloor(rf(30).l(), r3).ifb().fmul(rf(43).l(), r5, r1.h()).pushz()
		<< flpop(rf(22), rf(33)).fmul(rf(49).l(), r4.h(), r1.abs()).pushz()

		/* vfmul input packing */
		<< fmax(rf(46), r4.l(), r2.l()).nornn().vfmul(rf(45), r3, r5).ifnb()
		<< faddnf(r2.l(), r5.l(), r4).norc().vfmul(rf(15), r0.ll(), r4).ldunif().ifb()
		<< fcmp(rf(61).h(), r4.abs(), r2.l()).ifna().vfmul(rf(55), r2.hh(), r1)
		<< fsub(rf(27), r4.abs(), r1.abs()).norz().vfmul(rf(34), r3.swp(), r1).ifa()
		<< vfpack(rf(43), rf(15).l(), r0.h()).andnc().fmul(rf(10).h(), r4.l(), r5.abs()).ifna()
		<< fdx(rf(7).h(), r1.l()).ifnb().fmul(rf(46), r3.l(), r2.abs()).pushn()

		/* small immediates */
		<< vflb(rf(24)).andnn().fmul(rf(14), -8, rf(8).h())  // small imm index value '-8' is 24! This and 'rf(24)' confused me.
		<< vfmin(rf(24), si(15).ff(), r5).pushn().smul24(rf(15), r1, r3).ifnb()
		<< faddnf(rf(55), si(-16).l(), r3.abs()).pushc().fmul(rf(55).l(), rf(38).l(), r1.h()).ifb()
		<< fsub(rf(58).h(), si(0x3b800000).l(), r3.l()).nornc().fmul(rf(39), r0.h(), r0.h()).ifnb()

    /* branch conditions */
		<< bb(rf(19)).anyap()
		<< nop()
		<< bb(zero_addr+0xd0b76a28).anynaq()
		<< bb(lri).anynaq()
		<< bu(zero_addr+0x7316fe10, rf(35)).anya()
		<< bu(lri, r_unif).anynaq()
		<< bu(lri, a_unif).na0()

		/* Special waddr names */
		<< vfpack(tlb, r0, r1).nop()
		<< fmax(recip, r5.h(), r2.l()).andc().fmul(rf(50).h(), r3.l(), r4.abs()).ifb().ldunif()
		<< add(rsqrt, r1, r1).pushn().fmul(rf(35).h(), r3.abs(), r1.abs()).ldunif()
		<< vfmin(log, r4.hh(), r0).norn().fmul(rf(51), rf(20).abs(), r0.l()).ifnb()
		<< shl(exp, r3, r2).andn().add(rf(35), r1, r2).ifb()
		<< fsub(rf(26), r2.l(), rf(32)).ifa().fmul(sin, r1.h(), r1.abs()).pushc().ldunif()

		/* v4.1 signals */
		<< fcmp(rf(32), r2.h(), r1.h()).andz().vfmul(rf(20), r0.hh(), r3).ldunifa()
		<< fcmp(rf(38), r2.abs(), r5).fmul(rf(23).l(), r3, r3.abs()).ldunifarf(rf(1))
		<< fcmp(rf(52).h(), rf(23), r5.abs()).fmul(rf(16).h(), rf(23), r1).ldunifarf(rf(60))
		<< fmax(rf(43).l(), r3.h(), rf(30)).fmul(rf(35).h(), r4, r2.l()).ldunifarf(r1)
		<< faddnf(rf(7).l(), rf(28).h(), r1.l()).fmul(r1, r3.h(), r3.abs()).ldunifarf(rsqrt2)
		<< ffloor(rf(30).l(), r3).ifb().fmul(rf(43).l(), r5, r1.h()).pushz()

		/* v4.1 opcodes */
		<< ldvpmg_in(rf(7), r2, r2).mov(r3, 13)
		<< stvpmv(1, rf(8)).mov(r1, 1)
		<< sampid(rf(16)).fmul(rf(57).h(), r3, r1.l())
		<< brecip(rf(1), rf(62)).fmul(r3.h(), r2.l(), r1.l()).ldunifrf(rf(53))

		/* v4.1 SFU instructions. */
		<< brsqrt(rf(30), r4).fmul(rf(11), r4.h(), r2.h()).ldunifrf(rf(31))
		<< brsqrt2(rf(26), r3).fmul(rf(29).l(), r2.h(), r1.abs()).ldunifrf(rf(9))
		<< bsin(rf(8), rf(36)).fmul(rf(32), r2.h(), r0.l()).ldunifrf(rf(32))
		<< bexp(rf(20), r2).ifb().add(r2, rf(35), r2)
		<< blog(rf(63), r2).fmul(rf(34).h(), r4.l(), r1.abs()).andnn()

		/* v4.2 changes */
		<< barrierid(syncb).nop().thrsw()
	;

/*
	// Useful little code snippet for comparing expected and generated opcodes

	uint64_t op = 0x3c203192bb814000ull; //, "barrierid  syncb     ; nop               ; thrsw" },
	test_unpack_pack(op);
	Instr::show(op);
	auto tmp_op =
		barrierid(syncb).nop().thrsw()
	;
	tmp_op.dump(true);
*/

	ret << nop();

	std::vector<uint64_t> bytecode;
	for (auto const &instr : ret) {
		bytecode << instr.code(); 
	}

	return bytecode;
}
