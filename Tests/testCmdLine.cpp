#include "catch.hpp"
#include "../Lib/Support/Platform.h"


//
// get the base directory right for calling compiled apps.
//
#ifdef DEBUG
	#define POSTFIX_DEBUG "-debug"
#else
	#define POSTFIX_DEBUG ""
#endif

#ifdef QPU_MODE
//	#pragma message "QPU mode enabled"
	#define POSTFIX_QPU "qpu"
#else
	#define POSTFIX_QPU "emu"
#endif

#define BIN_PATH "obj/" POSTFIX_QPU POSTFIX_DEBUG "/bin"


//
// This test is fairly useless; it just checks if both the shell script and the C++ program
// fail or succeed at the same time.
//
// Better would be to check if the first line is the same
//
TEST_CASE("Detect platform scripts should both return the same thing", "[cmdline]") {
	int ret1 = system(BIN_PATH "/detectPlatform > /dev/null");
	bool success1 = (ret1 == 0);

	int ret2 = system("Tools/detectPlatform.sh > /dev/null");
	bool success2 = (ret2 == 0);

	INFO("C++ script returned " << ret1 << ", shell script returned " << ret2);
	REQUIRE(success1 == success2);
}


//
// This check of a single program serves as a canary for all generated output
//
TEST_CASE("ReqRecv check output and generation", "[cmdline]") {
	REQUIRE(!system("mkdir -p obj/test"));

	SECTION("Generated code should be as expected") {
		std::string output_filename   = "obj/test/ReqRecv_code.txt";
		std::string expected_filename = "Tests/data/ReqRecv_expected_code.txt";

		std::string cmdline = "cd obj/test && ../../" BIN_PATH "/ReqRecv ";  // NOTE: sudo needed for vc4
		cmdline +=  "-c -f > /dev/null";
		INFO("Cmdline: " << cmdline);
		REQUIRE(!system(cmdline.c_str()));

		std::string diff_cmd = "diff " + output_filename + " " + expected_filename;
		REQUIRE(!system(diff_cmd.c_str()));
	}

	SECTION("Interpreter should give correct output") {
		std::string output_filename   = "ReqRecv_output_int.txt";
		std::string expected_filename = "Tests/data/ReqRecv_expected_output.txt";

		std::string cmdline = BIN_PATH "/ReqRecv ";
		cmdline += "-r=interpreter > obj/test/" + output_filename;
		//printf("%s\n", cmdline.c_str());
		INFO("Cmdline: " << cmdline);
		REQUIRE(!system(cmdline.c_str()));

		std::string diff_cmd = "diff obj/test/" + output_filename + " " + expected_filename;
		REQUIRE(!system(diff_cmd.c_str()));
	}


	SECTION("Emulator should give correct output") {
		std::string output_filename   = "ReqRecv_output_emu.txt";
		std::string expected_filename = "Tests/data/ReqRecv_expected_output.txt";

		std::string cmdline = BIN_PATH "/ReqRecv ";
		cmdline +=  "-r=emulator > " + output_filename;
		INFO("Cmdline: " << cmdline);
		REQUIRE(!system(cmdline.c_str()));

		std::string diff_cmd = "diff obj/test/" + output_filename + " " + expected_filename;
		REQUIRE(!system(diff_cmd.c_str()));
	}
}

