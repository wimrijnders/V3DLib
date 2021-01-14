/******************************************************************************
 * NOTES
 * =====
 *
 * * Note 1: During testing, execution time shot up from 0.1 sec to 10 sec.
 *	         This probably due to the gpu hanging because of previously job
 *           faulty (too short). The 10 sec is likely the timeout.
 *		       TODO: Find a way to reset the v3d
 *
 * * Note 2: Following might be useful
 *
 *    #include <chrono>  // sleep_for()
 *    #include <thread>  // this_thread()
 *
 *    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
 *
 ******************************************************************************/
#ifdef QPU_MODE
#include <cstring>
#include <iostream>
#include "Common/SharedArray.h"
#include "v3d/v3d.h"
#include "v3d/instr/Instr.h"
#include "v3d/instr/Snippets.h"
#include "Support/Platform.h"
#include "Target/Syntax.h"   // mnemonics()
#include "support/support.h"
#include "support/summation_kernel.h"
#include "support/rotate_kernel.h"
#include "support/disasm_kernel.h"

#define ARRAY_LENGTH(arr, type) (sizeof(arr)/sizeof(type))

using BufferObject = V3DLib::v3d::BufferObject;

template<typename T>
using SharedArray = V3DLib::SharedArray<T>;

namespace {

using Instructions = V3DLib::v3d::Instructions;
using ByteCode = V3DLib::v3d::ByteCode;

const uint32_t DEFAULT_CODE_AREA_SIZE = 1024 * 1024;
const uint32_t DEFAULT_DATA_AREA_SIZE = 32 * 1024 * 1024;


// do_nothing is just the end-program sequence
uint64_t do_nothing[] = {
	0x3c203186bb800000, // nop; thrsw
	0x3c203186bb800000, // nop; thrsw
	0x3c003186bb800000, // nop
	0x3c003186bb800000, // nop
	0x3c203186bb800000, // nop; thrsw
	0x3c003186bb800000, // nop
	0x3c003186bb800000, // nop
	0x3c003186bb800000, // nop
};


//////////////////////////////////
// Test support routines
//////////////////////////////////

bool v3d_init() {
	if (!running_on_v3d()) {
		return false;
	}

	REQUIRE(v3d_open());
	return true;

}

}  // anon namespace



//////////////////////////////////
// The actual tests
//////////////////////////////////

TEST_CASE("Test v3d opcodes", "[v3d][code][opcodes]") {
	using namespace V3DLib::v3d::instr;
	using Instructions =  V3DLib::v3d::Instructions;

	if (!v3d_init()) return;

	// NOTE: Always uses rf(1) as dest ptr
	auto output = [] (Location const &src) -> Instructions {
		Instructions ret;
		ret << mov(tmud, src)          // write result to main mem
		    << mov(tmua, rf(1))
		    << tmuwt()  // does nothing?

		    << add(rf(1), rf(1), 4);

		ret.front().comment("Output to main mem");
		ret.back().comment("increment pointer");
		return ret;
	};


	auto mnemonics = [] (Instructions const &code, bool with_comments = false) -> std::string {
		std::string ret;

		for (int i = 0; i < code.size(); i++) {
			auto const &instr = code[i];
			ret << i << ": " << instr.mnemonic(with_comments) << "\n";
		}

		return ret;
	};


	/**
	 * Issues here:
	 * - sin via register always returns 1.0
	 * - ALU op's return nothing
	 */
	SECTION("Test SFU opcodes") {
		// This is bothersome
		// TODO find way to avoid qualifying namespaces
		auto sin =  V3DLib::v3d::instr::sin;
		auto exp =  V3DLib::v3d::instr::exp;
		auto log =  V3DLib::v3d::instr::log;

		Instructions instrs;

		instrs << nop().ldunifrf(rf(0))  // value to operate on
		       << nop().ldunifrf(rf(1))  // ptr to location to store

		// The classic way of using SFU:
		// - write to special register for function
		// - wait 2 cycles (for r4 to stabilize, can optimize with other instructions!)
		// - read result in r4

		       << mov(recip, rf(0))
		       << nop()
		       << nop()
		       << output(r4)

		       << mov(rsqrt, rf(0))
		       << nop()
		       << nop()
		       << output(r4)

		       << mov(exp, rf(0))    // base 2!
		       << nop()
		       << nop()
		       << output(r4)

		       << mov(log, rf(0))    // base 2!
		       << nop()
		       << nop()
		       << output(r4)

		       << mov(sin, rf(0))
		       << nop()
		       << nop()
		       << output(r4)

		       << mov(rsqrt2, rf(0))  // TODO what is difference with rsqrt?
		       << nop()
		       << nop()
		       << output(r4)

		// Using v3d ALU op's for the same
		// No output here!
		// TODO investigate further

		       << brecip(r1, rf(0))
		       << nop()
		       << nop()
		       << output(r1)

		       << brsqrt(r1, rf(0))
		       << nop()
		       << nop()
		       << output(r1)

		       << bexp(r1, rf(0))  // base 2!
		       << nop()
		       << nop()
		       << output(r1)

		       << blog(r1, rf(0))  // base 2!
		       << nop()
		       << nop()
		       << output(r1)

		       << bsin(r1, rf(0))
		       << nop()
		       << nop()
		       << output(r1)

		       << brsqrt2(r1, rf(0))
		       << nop()
		       << nop()
		       << output(r1)

		       << end_program();

		ByteCode bytecode;
		for (auto const &instrs : instrs) {
			bytecode << instrs.code(); 
		}

		BufferObject heap(1024);
		SharedArray<uint64_t> codeMem(bytecode.size(), heap);
		codeMem.copyFrom(bytecode);
		SharedArray<float> result(16, heap);
		SharedArray<uint32_t> unif(2, heap);

		// Some magic to store a float in a uint32_t
		float x = 2.5f;
  	int32_t *bits = (int32_t*) &x;
		unif[0] = *bits;

		unif[1] = result.getAddress();

		V3DLib::v3d::Driver driver;
		driver.add_bo(heap);
		REQUIRE(driver.execute(codeMem, &unif));

		dump_data(result, true, true);
		printf("\n");

		// TODO: add REQUIRE's for expected output
	}


	/**
	 * Added to investigate strange output with add/mov.
	 * Works now, is a canary
	 */
	SECTION("Test add/mov") {
		Instructions instrs;

		instrs << nop().ldunifrf(rf(0))       // value to operate on
		       << nop().ldunifrf(rf(1))       // ptr to location to store
		       << mov(r2, 4)                  // Prepare ptr increment value
		       << output(rf(0))               // Output original value

			     << add(rf(0), rf(0), 4)        // add small imm
		       << output(rf(0))

			     << add(rf(0), rf(0), r2)       // add via acc
		       << output(rf(0))

			     << add(r3, r2, rf(0))          // add first in acc, then move
		       << mov(rf(0), r3)
		       << output(rf(0))

			     << nop().mov(r2, 5)            // using mul alu for mov/add
			     << nop().add(rf(0), rf(0), r2)
		       << output(rf(0))

			     << nop().add(r3, r2, rf(0))    // using mul alu for add/mov
		       << nop().mov(rf(0), r3)
		       << output(rf(0))

		       << end_program();

		//printf("%s\n", mnemonics(instrs, true).c_str());

		ByteCode bytecode;
		for (auto const &instrs : instrs) {
			bytecode << instrs.code(); 
		}

		BufferObject heap(1024);
		SharedArray<uint64_t> codeMem(bytecode.size(), heap);
		codeMem.copyFrom(bytecode);
		SharedArray<int> result(16, heap);
		SharedArray<uint32_t> unif(2, heap);

		unif[0] = 123;
		unif[1] = result.getAddress();

		V3DLib::v3d::Driver driver;
		driver.add_bo(heap);
		REQUIRE(driver.execute(codeMem, &unif));

		//dump_data(result, true);
		//printf("\n");

		REQUIRE(result[0] == 123);
		REQUIRE(result[1] == 127);
		REQUIRE(result[2] == 131);
		REQUIRE(result[3] == 135);
		REQUIRE(result[4] == 140);
		REQUIRE(result[5] == 145);
	}
}


/*
 * Adjusted from: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
 */
TEST_CASE("Check v3d code is working properly", "[v3d][code]") {
	if (!v3d_init()) return;

	SECTION("Direct v3d calls should work with SharedArray") {
		using namespace V3DLib::v3d;

		uint32_t array_length = ARRAY_LENGTH(do_nothing, uint64_t);
		REQUIRE(array_length == 8);

		BufferObject heap(1024);
		//printf("heap phyaddr: %u, size: %u\n", heap.phy_address(), heap.size());

		SharedArray<uint64_t> codeMem(array_length, heap);
		//printf("codeMem phyaddr: %u, length: %u\n", codeMem.getAddress(), codeMem.size());
		codeMem.copyFrom(do_nothing, array_length);
		//dump_data(codeMem);

		// See Note 1
		//double start = get_time();
		Driver driver;
		driver.add_bo(heap);
		REQUIRE(driver.execute(codeMem));
		//double end = get_time();
		//printf("[submit done: %.6lf sec]\n", end - start);
	}

	SECTION("v3d SharedArray should work as expected") {
		const int SIZE = 16;

		SharedArray<uint32_t> arr(SIZE);
		REQUIRE(arr.size() == SIZE);

		arr.fill(0);
		for(int offset = 0; offset < SIZE; ++offset) {
			REQUIRE(arr[offset] == 0);
		}

		arr.fill(127);
		for(int offset = 0; offset < SIZE; ++offset) {
			REQUIRE(arr[offset] == 127);
		}

		arr.dealloc();
		REQUIRE(arr.size() == 0);
	}
}


TEST_CASE("Driver call for v3d should work", "[v3d][driver]") {
	if (!v3d_init()) return;

	SECTION("Summation example should work from bytecode") {
		uint8_t num_qpus = 8;  // Don't change these values! That's how the summation kernel bytecode
		int unroll_shift = 5;  // was compiled.

		run_summation_kernel(summation, num_qpus, unroll_shift);
	}


	SECTION("Summation example should work from kernel output") {
		uint8_t num_qpus = 8;
		int unroll_shift = 5;

		std::vector<uint64_t> data = summation_kernel(num_qpus, unroll_shift);
		run_summation_kernel(data, num_qpus, unroll_shift);

		// Try different parameters. The bytecode is different but results should be the same
		num_qpus = 1;
		unroll_shift = 1;
		data = summation_kernel(num_qpus, unroll_shift);
		run_summation_kernel(data, num_qpus, unroll_shift);
	}


	SECTION("Rotate example should work from bytecode") {
		run_rotate_alias_kernel(qpu_rotate_alias_code);
	}


	SECTION("Rotate example should work from kernel output") {
		run_rotate_alias_kernel(rotate_kernel());
	}
}


TEST_CASE("Check v3d rotate assembly/disassembly", "[v3d][asm]") {
	using namespace V3DLib::v3d::instr;

	SECTION("rotate kernel generates correctly encoded output") {
		std::vector<uint64_t> arr = rotate_kernel();
		REQUIRE(arr.size() > 0);

		match_kernel_outputs(qpu_rotate_alias_code, arr, true);
		REQUIRE(qpu_rotate_alias_code.size() == arr.size());
	}
}


TEST_CASE("Check v3d assembly/disassembly", "[v3d][asm]") {
	using namespace V3DLib::v3d::instr;

	SECTION("Correct output of dump program") {
		struct v3d_device_info devinfo;  // NOTE: uninitialized struct! For test OK
		devinfo.ver = 42;               //        <-- only this needs to be set

		const char *expected = "\n{\n\
  type: INSTR_TYPE_ALU,\n\
  sig: {ldunifrf },\n\
  sig_addr: 0,\n\
  sig_magic: false,\n\
  raddr_a: 0 ,\n\
  raddr_b: 0,\n\
  flags: {ac: COND_NONE, mc: COND_NONE, apf: PF_NONE, mpf: PF_NONE, auf: UF_NONE, muf: UF_NONE},\n\
  alu: {\n\
    add: NOP,\n\
    mul: NOP\n\
  }\n\
}\n";

		uint64_t nop = 0x3d803186bb800000;  // nop                  ; nop               ; ldunifrf.rf0 

		struct v3d_qpu_instr instr;
		REQUIRE(instr_unpack(&devinfo, nop, &instr));

		char buffer[10*1024];
		instr_dump(buffer, &instr);

		INFO("Expected:\n" << expected);
		INFO("Output:\n" << buffer);
		REQUIRE(!strcmp(expected, buffer));
	}


	SECTION("Summation kernel generates correctly encoded output") {
		auto arr = summation_kernel(8, 5, 0);
		REQUIRE(arr.size() > 0);

		//printf("%s\n", Instr::mnemonics(arr).c_str());

		match_kernel_outputs(summation, arr);
		REQUIRE(summation.size() == arr.size());
	}


	SECTION("Register without mux definition should throw on usage") {
		using namespace V3DLib::v3d::instr;

		REQUIRE_NOTHROW(r0.to_waddr());
		REQUIRE_NOTHROW(r0.to_mux());
		REQUIRE_NOTHROW(r1.to_waddr());
		REQUIRE_NOTHROW(r1.to_mux());

		// tmua has no mux usage
		REQUIRE_NOTHROW(tmua.to_waddr());
		REQUIRE_THROWS(tmua.to_mux());
	}


	SECTION("For opcode with two small immediates values, value should be the same") {
		REQUIRE_THROWS(shl(r0, 1, 5));
		REQUIRE_NOTHROW(shl(r3, 4, 4));
	}


	SECTION("Selected opcode should be encoded correctly") {
		using std::cout;
		using std::endl;
		//printf("Selected opcode should be encoded correctly\n");

		std::vector<std::string> expected = {
			"and  rf0, r0, 15     ; nop",
			"and  r1, r0, 15      ; nop"
		};

		std::vector<Instr> instrs; 

		instrs << band(rf(0), r0, 0b1111)
		       << band(r1, r0, 0b1111);

		for (int n = 0; n < instrs.size(); ++n) {
			//cout << instrs[n].mnemonic() << endl;
			REQUIRE(instrs[n].mnemonic() == expected[n]);
		}
	}


	SECTION("Opcode compare should work") {
		using namespace V3DLib::v3d::instr;

		// Non-branch instructions: direct compare
		REQUIRE(Instr::compare_codes(0x3d803186bb800000, 0x3d803186bb800000));  // nop-nop
		REQUIRE(!Instr::compare_codes(0x3d803186bb800000, 0x3c003181bb802000));  // nop-eidx

		// Special case: branch with field bu == false: ignore bdu field
		// This should succeed!
		REQUIRE(Instr::compare_codes(0x02ffeff3ff009000, 0x02ffeff3ff001000)); // 2x b.na0  -4112
	}


	SECTION("qpu_disasm kernel generates correctly encoded output") {
		auto &bytecode      = qpu_disasm_bytecode();
		auto  kernel_output = qpu_disasm_kernel();
		REQUIRE(kernel_output.size() > 0);

		match_kernel_outputs(bytecode, kernel_output, true);  // true: skip `nop` in kernel_output, can't generate
	}


	SECTION("Opcodes not in qpu_disasm kernel assembled correctly") {
		std::vector<Instr> ret;

		ret
			<< nop().smul24(r1, SmallImm(2), rf(0))
			<< rotate(r1, r0, r5)
			<< rotate(r1, r0, 3)
			<< shl(r3, 4, 4).mov(rf(1), r5)
		;

/*
		// Just eyeball them for now
		printf("Eyeballing opcodes:\n");
		for (auto &op : ret) {
			std::cout << op.mnemonic() << std::endl;
			//op.dump(true);
		}

		// Show last in full
		{
			auto &op = ret.back();

			std::cout << op.mnemonic() << std::endl;
			op.dump(true);
		}
*/
	}
}

#undef ARRAY_LENGTH

#endif  // QPU_MODE

