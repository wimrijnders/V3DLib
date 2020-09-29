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
bool const use_sudo = true;

#else
#define POSTFIX_QPU "emu"
const char *SUDO = "";
bool const use_sudo = false;

#endif

#define BIN_PATH "obj/" POSTFIX_QPU POSTFIX_DEBUG "/bin"

namespace {

enum RunType {
	QPU,
	EMULATOR,
	INTERPRETER
};


void check_output_run(std::string const &program, RunType run_type) {
	std::string params = "";
	std::string output_filename = "obj/test/";
	std::string expected_filename = "Tests/data/";

	output_filename   += program + "_";
	expected_filename += program + "_expected_output.txt";

	switch (run_type) {
		case QPU:
			output_filename += "qpu";
		break;
		case EMULATOR:
			params += "-r=emulator",
			output_filename += "emu";
		break;
		case INTERPRETER:
			params += "-r=interpreter",
			output_filename += "int";
		break;
	}

	output_filename   += "_output.txt";

	std::string cmdline = SUDO;
	cmdline += BIN_PATH "/";
	cmdline += program + " -silent " + params + " > " + output_filename;
	INFO("Cmdline: " << cmdline);
	REQUIRE(!system(cmdline.c_str()));

	std::string diff_cmd = "diff " + output_filename + " " + expected_filename;
	INFO("diff command: " << diff_cmd);
	REQUIRE(!system(diff_cmd.c_str()));
}


void make_test_dir() {
	std::string cmd = SUDO;
	cmd += "mkdir -p obj/test";

	REQUIRE(!system(cmd.c_str()));

	cmd  = SUDO;
	cmd += "chmod ugo+rw obj/test";
	REQUIRE(!system(cmd.c_str()));

	cmd  = SUDO;
	cmd += "chmod ugo+rw obj/test/*";
	REQUIRE(!system(cmd.c_str()));
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


TEST_CASE("ReqRecv check code generation", "[cmdline]") {
	make_test_dir();

	SECTION("Generated code should be as expected") {
		// WRI DEBUG disabled temporarily - I have doubts that this is useful (generated code a moving target)
/*

		std::string output_filename   = "obj/test/ReqRecv_code.txt";
		std::string expected_filename = "Tests/data/ReqRecv_expected_code.txt";

		std::string cmdline = "cd obj/test && ../../" BIN_PATH "/ReqRecv ";
		cmdline +=  "-r=emulator -c -f > /dev/null";
		INFO("Cmdline: " << cmdline);
		REQUIRE(!system(cmdline.c_str()));

		std::string diff_cmd = "diff " + output_filename + " " + expected_filename;
		INFO(diff_cmd);
		REQUIRE(!system(diff_cmd.c_str()));
*/
	}
}


TEST_CASE("Check correct output example programs for all three run options", "[cmdline]") {
	make_test_dir();

	SECTION("Check output ReqRecv") {
		check_output_run("ReqRecv", QPU);
		check_output_run("ReqRecv", INTERPRETER);
		check_output_run("ReqRecv", EMULATOR);
	}

	SECTION("Check output ID") {
		check_output_run("ID", QPU);
		check_output_run("ID", INTERPRETER);
		check_output_run("ID", EMULATOR);
	}

	SECTION("Check output Hello") {
		check_output_run("Hello", QPU);
		check_output_run("Hello", INTERPRETER);
		check_output_run("Hello", EMULATOR);
	}
}
