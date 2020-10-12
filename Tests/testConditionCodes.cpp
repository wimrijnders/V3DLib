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

void next(Ptr<Int> &result, Int &r) {
	*result = r;
	result += 16;
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
	Where (a >=  8) r = 1; End
	*result = r;
}

TEST_CASE("Test Where blocks", "[where][cond]") {
	const int SIZE      = 16;
	const int NUM_TESTS = 6;

  auto k = compile(where_kernel);

  QPULib::SharedArray<int> result(NUM_TESTS*SIZE);

	k.load(&result);
	//k.pretty(false);

	k.call();
	//k.emu();
	//k.interpret();

  //for (int i = 0; i < result.size(); i++)
  //  printf("%i: %i\n", i, result[i]);

	auto check = [&result] (int block, uint32_t *expected) {
		for (uint32_t n = 0; n < SIZE; ++n) {
			INFO("block " << block <<  ", index " << n);
			REQUIRE(result[block * SIZE + n] == expected[n]);
		}
	};

	uint32_t expected_smaller_than[SIZE]  = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_smaller_equal[SIZE] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_equal[SIZE]         = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_not_equal[SIZE]     = {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1};
	uint32_t expected_larger_than[SIZE]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};
	uint32_t expected_larger_equal[SIZE]  = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};

	check(0, expected_smaller_than);
	check(1, expected_smaller_equal);
	check(2, expected_equal);
	check(3, expected_not_equal);
	check(4, expected_larger_than);
	check(5, expected_larger_equal);
}

#endif  // ifdef QPU_MODE
