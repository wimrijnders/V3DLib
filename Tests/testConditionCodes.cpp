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
#include "Support/pgm.h"

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

	Where (a <  8)   r = 1; End; next(result, r);
	Where (a <= 8)   r = 1; End; next(result, r);
	Where (a == 8)   r = 1; End; next(result, r);
	Where (a != 8)   r = 1; End; next(result, r);
	Where (a >  8)   r = 1; End; next(result, r);
	Where (a >= 8)   r = 1; End; next(result, r);
	Where (!(a > 3)) r = 1; End; next(result, r);
	*result = r;
}


void andor_kernel(Ptr<Int> result) {
	Int a = index();
	Int r = 0;

	Where ( a >=  4 &&  a <= 8)             r = 1; End; next(result, r);
	Where ( a <   4 ||  a >  8)             r = 1; End; next(result, r);
	Where ( a >   4 &&  a <  8  || a > 12)  r = 1; End; next(result, r);
	Where ( a >   4 && (a <  8  || a > 12)) r = 1; End; next(result, r); // BORING! Same result as previous
	Where ((a >   4 &&  a <  8) || a > 12)  r = 1; End; next(result, r); // TODO find better examples with differing res

	Int b = index();
	Where ( a > 6 && a < 12  &&  b >  8 && b < 14)  r = 1; End; next(result, r);
	Where ( a > 6 && b < 14  &&  a < 12 && b >  8)  r = 1; End; next(result, r);
	Where ((a > 6 && a < 12) || (b >  8 && b < 14)) r = 1; End; next(result, r);

	Where ((a > 6 && a < 12) || (b >  8 && b < 14)) r = 1; Else r = 2;  End;

	*result = r;
}


void noloop_where_kernel(Ptr<Int> result, Int x, Int y) {
	Int tmp = 0;
	Where (y > 10 && y < 20 && x > 10 && x < 20)
		tmp = 1;
	End
	store(tmp, result);
}


void noloop_if_kernel(Ptr<Int> result, Int x, Int y) {
	Int tmp = 0;
	If (y > 10 && y < 20 && x > 10 && x < 20)
		tmp = 1;
	End
	store(tmp, result);
}


void noloop_multif_kernel(Ptr<Int> result, Int x, Int y) {
	Int tmp = 0;
	If (y > 10)
	If (y < 20)
	If (x > 10)
	If (x < 20)
		tmp = 1;
	End
	End
	End
	End
	store(tmp, result);
}


void andor_where_kernel(Ptr<Float> result, Int width, Int height) {
  For (Int y = 0, y < height, y = y + 1)
    Ptr<Float> p = result + y*width;  // Point p to the output row

    For (Int x = 0, x < width, x = x + VEC_SIZE)
			Float tmp = toFloat(1024);
			Where (y > 10 && y < 20 && x > 10 && x < 20)
				tmp = 0.0;
			End
      store(tmp, p);
      p = p + VEC_SIZE;
    End
  End
}


void andor_if_kernel(Ptr<Float> result, Int width, Int height) {
  For (Int y = 0, y < height, y = y + 1)
    Ptr<Float> p = result + y*width;  // Point p to the output row

    For (Int x = 0, x < width, x = x + VEC_SIZE)
			Float tmp = toFloat(1024);
			If (y > 10 && y < 20 && x > 10 && x < 20)
				tmp = 0.0;
			End
      store(tmp, p);
      p = p + VEC_SIZE;
    End
  End
}


void andor_multi_if_kernel(Ptr<Float> result, Int width, Int height) {
  For (Int y = 0, y < height, y = y + 1)
    Ptr<Float> p = result + y*width;  // Point p to the output row

    For (Int x = 0, x < width, x = x + VEC_SIZE)
			Float tmp = toFloat(1024);
			If (y > 10)
			If (y < 20)
			If (x > 10)
			If (x < 20)
				tmp = 0.0;
			End
			End
			End
			End
      store(tmp, p);
      p = p + VEC_SIZE;
    End
  End
}


void check(QPULib::SharedArray<int> &result, int block, uint32_t *expected) {
	for (uint32_t n = 0; n < VEC_SIZE; ++n) {
		INFO("block " << block <<  ", index " << n);

		std::string buf;

		for (int i = 0; i < VEC_SIZE; ++i) {
			buf << result[block * VEC_SIZE + i];
		}
		INFO("result  : " << buf);

		buf.clear();
		for (int i = 0; i < VEC_SIZE; ++i) {
			buf << expected[i];
		}
		INFO("expected: " << buf);

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
	uint32_t expected_not[VEC_SIZE]           = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	check(result, 0, expected_smaller_than);
	check(result, 1, expected_smaller_equal);
	check(result, 2, expected_equal);
	check(result, 3, expected_not_equal);
	check(result, 4, expected_larger_than);
	check(result, 5, expected_larger_equal);
	check(result, 6, expected_not);
}


void check_andor_result(QPULib::SharedArray<int> &result) {
	uint32_t expected_and[VEC_SIZE]           = {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_or[VEC_SIZE]            = {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};
	uint32_t expected_combined[VEC_SIZE]      = {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1};
	uint32_t expected_multi_and[VEC_SIZE]     = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0};
	uint32_t expected_multi_andor[VEC_SIZE]   = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0};
	uint32_t expected_multi_else[VEC_SIZE]    = {2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2};

	check(result, 0, expected_and);
	check(result, 1, expected_or);
	check(result, 2, expected_combined);
	check(result, 3, expected_combined);
	check(result, 4, expected_combined);
	check(result, 5, expected_multi_and);
	check(result, 6, expected_multi_and);
	check(result, 7, expected_multi_andor);
	check(result, 8, expected_multi_else);
}


void reset(QPULib::SharedArray<int> &result, int val = 0) {
  for (int i = 0; i < result.size(); i++) { result[i] = val; }
}


void reset(QPULib::SharedArray<float> &result, float val = 0.0) {
  for (int i = 0; i < result.size(); i++) { result[i] = val; }
}


void check_pgm(std::string const &filename) {
	std::string expected_filename = "Tests/data/where_expected.pgm";

	std::string diff_cmd = "diff " + filename + " " + expected_filename;
	INFO("diff command: " << diff_cmd);
	REQUIRE(!system(diff_cmd.c_str()));
}


}  // anon namespace


TEST_CASE("Test Where blocks", "[where][cond]") {
	int const NUM_TESTS = 7;

  auto k = compile(where_kernel);

  QPULib::SharedArray<int> result(NUM_TESTS*VEC_SIZE);

	k.load(&result);

	reset(result);
	k.emu();
	check_where_result(result);

	reset(result);
	k.interpret();
	check_where_result(result);

	reset(result);
	k.call();
	check_where_result(result);

  //for (int i = 0; i < result.size(); i++)
  //  printf("%i: %i\n", i, result[i]);
}


/**
 * This is meant as a precursor for the following test,
 * to ensure that the contents of the double loops work as expected
 */
TEST_CASE("Test if/where without loop", "[noloop][cond]") {
	uint32_t expected_1[VEC_SIZE]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint32_t expected_2[VEC_SIZE]  = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  auto k1 = compile(noloop_where_kernel);
	k1.pretty(false,  "obj/test/noloop_where_v3d.txt");	
  auto k2 = compile(noloop_if_kernel);
  auto k3 = compile(noloop_multif_kernel);

  QPULib::SharedArray<int> result(VEC_SIZE);

	auto run_all = [&result] (decltype(k1) &k, uint32_t *expected) {
		reset(result, -1);
		k.emu();
		check(result, 0, expected);

		reset(result, -1);
		k.interpret();
		check(result, 0, expected);

		reset(result, -1);
		k.call();
		check(result, 0, expected);
	};

	k1.load(&result, 0, 0);   run_all(k1, expected_1);
	k1.load(&result, 12, 15); run_all(k1, expected_2);
//	k.load(&result, 21, 15); run_all(k, expected_1);  // !!! timer expired on v3d!

	k2.load(&result, 0, 0);   run_all(k2, expected_1);
	k2.load(&result, 12, 15); run_all(k2, expected_2);

	k3.load(&result, 0, 0);   run_all(k3, expected_1);
	k3.load(&result, 12, 15); run_all(k3, expected_2);
}


TEST_CASE("Test multiple and/or", "[andor][cond]") {
	SECTION("Test Where blocks with and/or") {
		const int NUM_TESTS = 9;

	  auto k = compile(andor_kernel);

	  QPULib::SharedArray<int> result(NUM_TESTS*VEC_SIZE);

		k.load(&result);

		reset(result);
		k.emu();
		check_andor_result(result);

		reset(result);
		k.interpret();
		check_andor_result(result);

		reset(result);
		k.call();
		check_andor_result(result);
	}

	SECTION("Test Where blocks with multiple and/or") {
		make_test_dir();
		int const width  = 48;
		int const height = 32;

	  QPULib::SharedArray<float> result(width*height);

		auto check_output = [width, height, &result] (char const *label) {
			std::string filename;
			filename <<  "obj/test/andor_" << label << "_output.pgm";
			output_pgm_file(result, width, height, 255, filename.c_str());
			check_pgm(filename.c_str());
		};

		auto k1 = compile(andor_where_kernel);
		k1.load(&result, width, height);

		reset(result);
		k1.emu();
		check_output("where_emu");

		reset(result);
		k1.interpret();
		check_output("where_int");


		if (Platform::instance().has_vc4) {
			std::cout << "Not running the 'where_qpu' test on vc4; this hangs the pi4 (TODO)" << std::endl;
		} else {
			// v3d: works fine
			reset(result);
			k1.call();
			check_output("where_qpu");
		}


		k1.pretty(false,  "obj/test/andor_where_v3d.txt");	


		auto k2 = compile(andor_if_kernel);
		k2.load(&result, width, height);

		k2.pretty(true,  "obj/test/andor_if_vc4.txt");	
		k2.pretty(false, "obj/test/andor_if_v3d.txt");	

		reset(result);
		k2.emu();
		check_output("if_emu");

		reset(result);
		k2.interpret();
		check_output("if_int");

		std::cout << "Not running the 'if_qpu' test; this hangs the pi4 and locks up the videocore on pi3 (TODO)" << std::endl;
/*
		// vc4: hangs the pi4
		// v3d: fails, no output. Locks the videocore, needs reset
		reset(result);
		k2.call();
		check_output("if_qpu");
*/


		auto k3 = compile(andor_multi_if_kernel);
		k3.load(&result, width, height);

		reset(result);
		k3.emu();
		check_output("multi_if_emu");

		reset(result);
		k3.interpret();
		check_output("multi_if_int");

		k3.pretty(false, "obj/test/andor_multi_if_v3d.txt");	

		std::cout << "Not running the 'multi_if_qpu' test; this hangs the pi4 and locks up the videocore on pi3 (TODO)" << std::endl;
/*
		// vc4: hangs the pi4
		// v3d: fails, no output. Locks the videocore, needs reset
		reset(result);
		k2.call();
		check_output("multi_if_qpu");
*/
	}
}



#endif  // ifdef QPU_MODE
