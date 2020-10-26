//
// Example(s) here adaptted from: https://github.com/Idein/py-videocore6/blob/master/tests/test_condition_codes.py
//
// TODO perhaps translate rest of tests
//
// ============================================================================
// NOTES
// =====
//
// * This is how it appears to work:
//
//   - There are two condition flags a,b
//   - Using `push[znc]` will move current value of a to b and sets b
//   - `if[n]a` checks value of flag a, `if[n]b` checks b. The 'n' means 'not,
//     thus is true when a/b are 0.
//
// * In the test, `pushn` and `pushc` appear to do the same thing!
//
// * Condition code handling is different vor `vc4` and `v3d`:
//
//   - vc4: Each index element has associated vectors of status bits: Z,N,C
//          There is one such bit vector for the add alu and one for the mul alu
//          If field `sf (setFlags)` is set in an instruction, these are all
//          set depending on the result of the result of the instruction
//   - v3d: There are two condition bits per index element: a, b
//          There is an a/b bit for the add alu and the same set for the mul alu
//          The instruction explicitly states which condition should be tested: Z,N,C
//          The result of the condition goes into a. The previous value of a goes into b
//
///////////////////////////////////////////////////////////////////////////////
#ifdef QPU_MODE
#include <iostream>
#include "support/support.h"

namespace {

using namespace QPULib::v3d::instr;
using Instructions = std::vector<Instr>; 

/**
 * `cond = 'push*'` sets the conditional flag A
 */
ByteCode qpu_cond_push_a() {
	Instructions ret;

	auto set_cond_push = [] (Instr &instr, int index) {
		// Set a-flag with given condition
		switch (index) {
			case 0: instr.pushz(); break;  // == 0
			case 1: instr.pushn(); break;  // < 0  ? Appear to do the same thing
			case 2: instr.pushc(); break;  // < 0
		}
	}
;
	auto set_cond_if = [] (Instr &instr, int index) {
		switch (index) {
			case 0: instr.ifa();  break;  // Test if set
			case 1: instr.ifna(); break;  // Test if not set
			case 2: instr.ifa();  break;
		}
	};

	// r2 = ptr + index*4 
	ret << eidx(r0).ldunif()   // Loads uniform value in r5
	    << mov(r2, r5)
	    << shl(r0, r0, 2)
	    << add(r2, r2, r0)
	    << shl(r1, 4, 4);      // r1 = 64 (offset)

	for (int index = 0; index < 3; ++ index) {
		ret << eidx(r0);

		Instr instr = sub(r0, r0, 10);  // r0 = index - 10
		set_cond_push(instr, index);

		ret << instr
		    << mov(r0, SmallImm(0));

		instr = mov(r0, SmallImm(1));
		set_cond_if(instr, index);

		ret << instr
		    << mov(tmud, r0)
		    << mov(tmua, r2)
		    << tmuwt().add(r2, r2, r1)  // *ptr = val; ptr += 64
		    << mov(r0, SmallImm(0));

		instr = nop().mov(r0, SmallImm(1));
		set_cond_if(instr, index);

		ret << instr
		    << mov(tmud, r0)
		    << mov(tmua, r2)
		    << tmuwt().add(r2, r2, r1);
	}

	ret << nop().thrsw()
	    << nop().thrsw()
	    << nop()
	    << nop()
	    << nop().thrsw()
	    << nop()
	    << nop()
	    << nop();

	ByteCode bytecode;
	for (auto const &instr : ret) {
		bytecode << instr.code(); 
	}

	return bytecode;
}


void dump_code(ByteCode &code) {
	for (uint32_t offset = 0; offset < code.size(); ++offset) {
		std::cout << Instr::mnemonic(code[offset]) << "\n";
	}

	std::cout << std::endl;
}

}  // anon namespace


TEST_CASE("Check v3d condition codes", "[v3d][cond]") {
	using namespace QPULib::v3d;

	SECTION("Test condition push a") {
		if (!running_on_v3d()) return;
		const int DATA_SIZE = 16;

		ByteCode bytecode = qpu_cond_push_a();
		BufferObject heap(10*1024);  // arbitrary size, large enough
		//dump_code(bytecode);

		SharedArray<uint64_t> code(bytecode.size(), heap);
		code.copyFrom(bytecode);

		SharedArray<uint32_t> data(6*DATA_SIZE, heap);
		for (uint32_t offset = 0; offset < data.size(); ++offset) {
			data[offset] = 0;
		}

		SharedArray<uint32_t> unif(1, heap);
		unif[0] = data.getAddress();

		QPULib::v3d::Driver drv;
		drv.add_bo(heap);
		REQUIRE(drv.execute(code, &unif));

		//dump_data(data, true); 

		uint32_t pushz_if_expected[DATA_SIZE]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 };
		uint32_t pushz_ifn_expected[DATA_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 };
		uint32_t pushc_if_expected[DATA_SIZE]  = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };


		for (uint32_t n = 0; n < DATA_SIZE; ++n) {
			INFO("block 0-1, index " << n);
			REQUIRE(data[0 * DATA_SIZE + n] == pushz_if_expected[n]);
			REQUIRE(data[1 * DATA_SIZE + n] == pushz_if_expected[n]);
		}
		for (uint32_t n = 0; n < DATA_SIZE; ++n) {
			INFO("block 2-3, index " << n);
			REQUIRE(data[2 * DATA_SIZE + n] == pushz_ifn_expected[n]);
			REQUIRE(data[3 * DATA_SIZE + n] == pushz_ifn_expected[n]);
		}
		for (uint32_t n = 0; n < DATA_SIZE; ++n) {
			INFO("block 4-5, index " << n);
			REQUIRE(data[4 * DATA_SIZE + n] == pushc_if_expected[n]);
			REQUIRE(data[5 * DATA_SIZE + n] == pushc_if_expected[n]);
		}
	}
}


#include "QPULib.h"
using namespace QPULib;

namespace {
const int VEC_SIZE = 16;

void next(Ptr<Int> &result, Int &r) {
	*result = r;
	result += VEC_SIZE;
	r = 0;
}

void where_kernel(Ptr<Int> result) {
	Int a = index();
	Int r = 0;

	Where (a <  8) r = 1; End; next(result, r);
	Where (a <= 8) r = 1; End; next(result, r);
	Where (a == 8) r = 1; End; next(result, r);
	Where (a != 8) r = 1; End; next(result, r);
	Where (a >  8) r = 1; End; next(result, r);
	Where (a >=  8) r = 1; End; next(result, r);
	*result = r;
}


void andor_kernel(Ptr<Int> result) {
	Int a = index();
	Int r = 0;

	Where ( a >=  4 &&  a <= 8)             r = 1; End; next(result, r);
	Where ( a <   4 ||  a >  8)             r = 1; End; next(result, r);
	Where ( a >   4 &&  a <  8  || a > 12)  r = 1; End; next(result, r);
	Where ( a >   4 && (a <  8  || a > 12)) r = 1; End; next(result, r); // BORING! Same result as previous
	Where ((a >   4 &&  a <  8) || a > 12)  r = 1; End                   // TODO find better examples with differing res

	*result = r;
}


void check(QPULib::SharedArray<int> &result, int block, uint32_t *expected) {
	for (uint32_t n = 0; n < VEC_SIZE; ++n) {
		INFO("block " << block <<  ", index " << n);
		REQUIRE(result[block * VEC_SIZE + n] == expected[n]);
	}
}


void check_where_result(QPULib::SharedArray<int> &result) {

	uint32_t expected_smaller_than[VEC_SIZE]  = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_smaller_equal[VEC_SIZE] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_equal[VEC_SIZE]         = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_not_equal[VEC_SIZE]     = {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1};
	uint32_t expected_larger_than[VEC_SIZE]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};
	uint32_t expected_larger_equal[VEC_SIZE]  = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};

	check(result, 0, expected_smaller_than);
	check(result, 1, expected_smaller_equal);
	check(result, 2, expected_equal);
	check(result, 3, expected_not_equal);
	check(result, 4, expected_larger_than);
	check(result, 5, expected_larger_equal);
}


void check_andor_result(QPULib::SharedArray<int> &result) {

	uint32_t expected_and[VEC_SIZE]           = {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_or[VEC_SIZE]            = {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};
	uint32_t expected_combined[VEC_SIZE]      = {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1};

	check(result, 0, expected_and);
	check(result, 1, expected_or);
	check(result, 2, expected_combined);
	check(result, 3, expected_combined);
	check(result, 4, expected_combined);
}

}  // anon namespace


TEST_CASE("Test Where blocks", "[where][cond]") {
	const int NUM_TESTS = 6;

  auto k = compile(where_kernel);

  QPULib::SharedArray<int> result(NUM_TESTS*VEC_SIZE);

	k.load(&result);
	//k.pretty(false);

  for (int i = 0; i < result.size(); i++) { result[i] = 0; }
	k.emu();
	check_where_result(result);

  for (int i = 0; i < result.size(); i++) { result[i] = 0; }
	k.interpret();
	check_where_result(result);

  for (int i = 0; i < result.size(); i++) { result[i] = 0; }
	k.call();
	check_where_result(result);

  //for (int i = 0; i < result.size(); i++)
  //  printf("%i: %i\n", i, result[i]);
}


TEST_CASE("Test Where blocks with and/or", "[andor][cond]") {
	const int NUM_TESTS = 5;

  auto k = compile(andor_kernel);

  QPULib::SharedArray<int> result(NUM_TESTS*VEC_SIZE);

	k.load(&result);

  for (int i = 0; i < result.size(); i++) { result[i] = 0; }
	k.emu();
	check_andor_result(result);

  for (int i = 0; i < result.size(); i++) { result[i] = 0; }
	k.interpret();
	check_andor_result(result);

/*
  for (int i = 0; i < result.size(); i++) { result[i] = 0; }
	k.call();
	check_andor_result(result);
*/
}

#endif  // ifdef QPU_MODE
