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
const char *SUDO = (Platform::instance().has_vc4)? "sudo " : "";  // sudo needed for vc4

#else
#define POSTFIX_QPU "emu"
const char *SUDO = "";

#endif

#define BIN_PATH "obj/" POSTFIX_QPU POSTFIX_DEBUG "/bin"

namespace {

void check_output_run_ReqRecv(
	std::string const &params,
	std::string const &output_filename,
	std::string const &expected_filename) {
	std::string cmdline = SUDO;
	cmdline += BIN_PATH "/ReqRecv ";
	cmdline += params + " > " + output_filename;
	INFO("Cmdline: " << cmdline);
	REQUIRE(!system(cmdline.c_str()));

	std::string diff_cmd = "diff " + output_filename + " " + expected_filename;
	REQUIRE(!system(diff_cmd.c_str()));
}

}  // anon namespace

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

		std::string cmdline = "cd obj/test && ../../" BIN_PATH "/ReqRecv ";
		cmdline +=  "-r=emulator -c -f > /dev/null";
		INFO("Cmdline: " << cmdline);
		REQUIRE(!system(cmdline.c_str()));

		std::string diff_cmd = "diff " + output_filename + " " + expected_filename;
		INFO(diff_cmd);
		REQUIRE(!system(diff_cmd.c_str()));
	}

	SECTION("Interpreter should give correct output") {
		check_output_run_ReqRecv(
			"-r=interpreter",
			"obj/test/ReqRecv_output_int.txt",
			"Tests/data/ReqRecv_expected_output.txt");
	}

	SECTION("Emulator should give correct output") {
		check_output_run_ReqRecv(
			"-r=emulator",
			"obj/test/ReqRecv_output_emu.txt",
			"Tests/data/ReqRecv_expected_output.txt");
	}

	SECTION("QPU should give correct output") {
		if (!Platform::instance().has_vc4) return;  // TODO: enable for v3d when assembly completed

		check_output_run_ReqRecv(
			"",
			"obj/test/ReqRecv_output_qpu.txt",
			"Tests/data/ReqRecv_expected_output.txt");
	}
}

