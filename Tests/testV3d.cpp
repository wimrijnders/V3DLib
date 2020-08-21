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
#include "catch.hpp"
#ifdef QPU_MODE
#include <sys/time.h>
#include <cstring>
#include <iostream>
#include "v3d/SharedArray.h"
#include "v3d/v3d.h"
#include "v3d/instr/Instr.h"
#include "v3d/Driver.h"
#include "../Lib/Support/basics.h"
#include "Support/debug.h"
#include "Support/Platform.h"
#include "support/summation.h"
#include "support/disasm_kernel.h"

#define ARRAY_LENGTH(arr, type) (sizeof(arr)/sizeof(type))

using BufferObject = QPULib::v3d::BufferObject;


namespace {

const uint32_t DEFAULT_CODE_AREA_SIZE = 1024 * 1024;
const uint32_t DEFAULT_DATA_AREA_SIZE = 32 * 1024 * 1024;


// Note that do_nothing is an array of 64-bit values!
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

void check_returned_registers(QPULib::v3d::SharedArray<uint32_t> &Y) {
	uint32_t cur_QPU;

	for (uint32_t offset = 0; offset < Y.size(); ++offset) {
		uint32_t this_QPU = offset / 16;

		if (this_QPU != cur_QPU) {
			printf("\n");
			cur_QPU = this_QPU;
			printf("%u: ", cur_QPU);	
		} 

		bool used = Y[offset] != 0;
		if (used) {
			printf("%u, ", offset % 16);	
		}
	}

	printf("\n");
}


double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + t.tv_usec * 1e-6;
}


template<typename T>
void dump_data(T const &arr, bool do_all = false) {
	int const DISP_LENGTH = 4;
	char const *format = "%8d: 0x%x - %d\n";

	if (sizeof(arr[0]) == 8) {
		format = "%8d: 0x%llx - %lld\n";
	}


	int first_size = (int) arr.size();

	if (do_all) {
		for (int offset = 0; offset < first_size; ++offset) {
			printf(format, offset, arr[offset], arr[offset]);
		}
		return;
	}


	if (first_size > DISP_LENGTH) {
		first_size = DISP_LENGTH;
	}
		
	for (int offset = 0; offset < first_size; ++offset) {
		printf(format, offset, arr[offset], arr[offset]);
	}

	if (first_size == arr.size()) {
		return;
	}

	printf("      ...\n");

	for (int offset = arr.size() - DISP_LENGTH; offset < (int) arr.size(); ++offset) {
		printf(format, offset, arr[offset], arr[offset]);
	}

	printf("\n");
}


/**
 * @param skip_nops  If true, don't compare nops in received output.
 *                   This indicates instructions which can't be generated to bytecode
 */
void match_kernel_outputs(
	std::vector<uint64_t> const &expected,
	std::vector<uint64_t> const &received,
	bool skip_nops = false) {
		using namespace QPULib::v3d::instr;
		auto _nop = nop();

		// Arrays should eventually match exactly, including length
		// For now, just check progress
		uint32_t len = expected.size();
		if (len > received.size()) {
			len = received.size();
		}

		// Outputs should match exactly
		for (uint32_t n = 0; n < len; ++n) {
			if (skip_nops && (_nop == received[n])) {
				continue;
			}

			INFO("Comparing assembly index: " << n << ", code length: " << received.size() <<
				"\nExpected: " << Instr(expected[n]).dump() <<
				"Received: " << Instr(received[n]).dump()
			);

			REQUIRE(Instr::compare_codes(expected[n], received[n]));
		}
}


//////////////////////////////////
// v3d routines
//////////////////////////////////

bool v3d_init() {
	static bool did_first = false;

	// Skip test if not on Pi4
	if (Platform::instance().has_vc4) {
		if (!did_first) {
			printf("Skipping v3d tests with calls to driver\n");
			did_first = true;
		}
		return false;
	}

	REQUIRE(v3d_open());
	return true;
}


//
// Adapted from: https://github.com/Idein/py-videocore6/blob/master/examples/summation.py
//
// This uses a single shared array for code and data.
// It might be possible to use muliple arrays, but we're sticking to the original
// example here.
//
void run_summation_kernel(std::vector<uint64_t> &bytecode, uint8_t num_qpus, int unroll_shift) {
	using namespace QPULib::v3d;

	printf("bytecode size: %u\n", bytecode.size());

	REQUIRE((num_qpus == 1 || num_qpus == 8));

	uint32_t length = 32 * 1024 * 16;  // Highest number without overflows in 8  QPU's and CPU
	                                   // The python version went to 32*1024*1024 and did some modulo magic.

	if (num_qpus == 1) {
		length = 32 * 1024 * 8;  // Highest number without overflows for 1 QPU
	}

	REQUIRE(length > 0);
	REQUIRE(length % (16 * 8 * num_qpus * (1 << unroll_shift)) == 0);


	printf("==== summation example (%dK elements) ====\n", (length / 1024));

	// Code and data is combined in one buffer
	uint32_t code_area_size = 8*bytecode.size();  // size in bytes
	printf("code_area_size size: %u\n", code_area_size);
	uint32_t data_area_size = (length + 1024) * 4;
	printf("data_area_size size: %u\n", data_area_size);

	BufferObject heap(code_area_size + data_area_size);
	printf("heap phyaddr: %u, size: %u\n", heap.getPhyAddr(), heap.size());

		heap.fill(0xdeadbeef);

		SharedArray<uint64_t> code(bytecode.size(), heap);
		code.copyFrom(bytecode);
		printf("code phyaddr: %u, size: %u\n", code.getPhyAddr(), 8*code.size());
		dump_data(code); 

		SharedArray<uint32_t> X(length, heap);
		SharedArray<uint32_t> Y(16 * num_qpus, heap);
		printf("X phyaddr: %u, size: %u\n", X.getPhyAddr(), 4*X.size());
		printf("Y phyaddr: %u, size: %u\n", Y.getPhyAddr(), 4*Y.size());

		auto sumY = [&Y] () -> uint64_t {
			uint64_t ret = 0;

			for (uint32_t offset = 0; offset < Y.size(); ++offset) {
				ret += Y[offset];
			}

			return ret;
		};

		for (uint32_t offset = 0; offset < X.size(); ++offset) {
			X[offset] = offset;
		}
		dump_data(X); 

		for (uint32_t offset = 0; offset < Y.size(); ++offset) {
			Y[offset] = 0;
		}
		dump_data(Y); 
		REQUIRE(sumY() == 0);

		SharedArray<uint32_t> unif(3, heap);
		unif[0] = length;
		unif[1] = X.getPhyAddr();
		unif[2] = Y.getPhyAddr();
		printf("unif phyaddr: %u, size: %u\n", unif.getPhyAddr(), 4*unif.size());


		printf("Executing on QPU...\n");
		double start = get_time();

		QPULib::v3d::Driver drv;
		drv.add_bo(heap);
		drv.execute(code, &unif, num_qpus);

		dump_data(Y, true);
		check_returned_registers(Y);
		heap.detect_used_blocks();

/*
		// Check if code not overwritten
		for (uint32_t offset = 0; offset < summation.size(); ++offset) {
			INFO("Code offset: " << offset);
			REQUIRE(code[offset] == summation[offset]);
		}

		// Check if X not overwritten
		for (uint32_t offset = 0; offset < X.size(); ++offset) {
			INFO("X offset: " << offset);
			REQUIRE(X[offset] == offset);
		}

		heap.find_value(1736704u); // 4278190080u;
*/
	
		// Check if values supplied
    REQUIRE(sumY()  == 1llu*(length - 1)*length/2);
		
		double end = get_time();
		printf("Summation done: %.6lf sec, %.6lf MB/s\n", (end - start), (length * 4 / (end - start) * 1e-6));
}

}  // anon 


/*
 * Adjusted from: https://gist.github.com/notogawa/36d0cc9168ae3236902729f26064281d
 */
TEST_CASE("Check v3d code is working properly", "[v3d]") {
	SECTION("Direct v3d calls should work with SharedArray") {
		using namespace QPULib::v3d;

		if (!v3d_init()) return;

		uint32_t array_length = ARRAY_LENGTH(do_nothing, uint64_t);
		assert(array_length == 8);

		BufferObject heap(1024);
		printf("heap phyaddr: %u, size: %u\n", heap.getPhyAddr(), heap.size());

		SharedArray<uint64_t> codeMem(array_length, heap);
		printf("codeMem phyaddr: %u, length: %u\n", codeMem.getPhyAddr(), codeMem.size());
		codeMem.copyFrom(do_nothing, array_length);
		dump_data(codeMem);

		// See Note 1
		double start = get_time();
		Driver driver;
		driver.add_bo(heap);
		REQUIRE(driver.execute(codeMem));
		double end = get_time();
		printf("[submit done: %.6lf sec]\n", end - start);
	}


	SECTION("v3d SharedArray should work as expected") {
		if (!v3d_init()) return;

		const int SIZE = 16;

		QPULib::v3d::SharedArray<uint32_t> arr(SIZE);
		REQUIRE(arr.size() == SIZE);

		for(int offset = 0; offset < SIZE; ++offset) {
			arr[offset] = 0;
		}

		for(int offset = 0; offset < SIZE; ++offset) {
			REQUIRE(arr[offset] == 0);
		}

		for(int offset = 0; offset < SIZE; ++offset) {
			arr[offset] = 127;
		}

		for(int offset = 0; offset < SIZE; ++offset) {
			REQUIRE(arr[offset] == 127);
		}

		arr.dealloc();
		REQUIRE(arr.size() == 0);
	}
}


TEST_CASE("Driver call for v3d should work", "[v3d][driver]") {
	SECTION("Summation example should work from bytecode") {
		if (!v3d_init()) return;

		uint8_t num_qpus = 8;  // Don't change these value! That's how the summation kernel bytecode
		int unroll_shift = 5;  // was compiled.

		run_summation_kernel(summation, num_qpus, unroll_shift);
	}


	SECTION("Summation example should work from kernel output") {
		if (!v3d_init()) return;

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
}


TEST_CASE("Check v3d assembly/disassembly", "[v3d][asm]") {
	using namespace QPULib::v3d::instr;

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
		std::vector<uint64_t> arr = summation_kernel(8, 5, 0);
		REQUIRE(arr.size() > 0);

		match_kernel_outputs(summation, arr);
		REQUIRE(summation.size() == arr.size());
	}


	SECTION("Register without mux definition should throw on usage") {
		using namespace QPULib::v3d::instr;

		REQUIRE_NOTHROW(r0.to_waddr());
		REQUIRE_NOTHROW(r0.to_mux());
		REQUIRE_NOTHROW(r1.to_waddr());
		REQUIRE_NOTHROW(r1.to_mux());

		// tmua has no mux usage
		REQUIRE_NOTHROW(tmua.to_waddr());
		REQUIRE_THROWS(tmua.to_mux());
	}


	SECTION("Selected opcode should be encoded correctly") {
		using std::cout;
		using std::endl;
		printf("Selected opcode should be encoded correctly\n");

		std::vector<std::string> expected = {
			"and  rf0, r0, 15     ; nop",
			"and  r1, r0, 15      ; nop"
		};

		std::vector<Instr> instrs; 

		instrs << band(rf(0), r0, 0b1111)
		       << band(r1, r0, 0b1111);

		for (int n = 0; n < instrs.size(); ++n) {
			cout << instrs[n].mnemonic() << endl;
			REQUIRE(instrs[n].mnemonic() == expected[n]);
		}
	}


	SECTION("Opcode compare should work") {
		using namespace QPULib::v3d::instr;

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
		//REQUIRE(summation.size() == arr.size());
	}
}

#undef ARRAY_LENGTH

#endif  // QPU_MODE

