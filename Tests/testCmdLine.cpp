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


namespace {

void ssystem(const char * cmd) {
           char buf[BUFSIZ];
           FILE *ptr; //, *file;

   //        file = fopen("/some/file", "w");
 //          if (!file) abort();
           if ((ptr = popen(cmd, "r")) != NULL) {
                   while (fgets(buf, BUFSIZ, ptr) != NULL)
     //                      fprintf(file, "%s", buf);
                           printf("out: %s\n", buf);
                   pclose(ptr);
           } else {
printf("Command failed/n");
           }

//           fclose(file);
//           return 0;
   }
}


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

	const char *BASE_CMDLINE      = "cd obj/test && ../../" BIN_PATH "/ReqRecv ";  // sudo needed for vc4
	std::string expected_filename = "Tests/data/ReqRecv_expected_output.txt";

	SECTION("Generated code should be as expected") {
		std::string cmdline = BASE_CMDLINE;
		cmdline +=  "-c -f > /dev/null";
		INFO("Cmdline: " << cmdline);
//		REQUIRE(!system(cmdline.c_str()));

		std::string expected_filename;
//		if (Platform::instance().emulator_only) {
//			// TODO: This is wrong! There should be no difference in output between emu and vc4.
//			expected_filename = "Tests/data/ReqRecv_expected_emu.txt";
//		} else {
			expected_filename = "Tests/data/ReqRecv_expected_vc4.txt";
//		}

		std::string diff_cmd = "diff obj/test/ReqRecv_code.txt " + expected_filename;
		REQUIRE(!system(diff_cmd.c_str()));
	}

	SECTION("Interpreter should give correct output") {
		const char *BASE_CMDLINE      = "sudo " BIN_PATH "/ReqRecv ";  // sudo needed for vc4
		std::string output_filename   = "ReqRecv_output_int.txt";
		std::string cmdline = BASE_CMDLINE;
		cmdline += "-r=interpreter"; // > obj/test/" + output_filename;
		printf(cmdline.c_str());
		INFO("Cmdline: " << cmdline);
		ssystem(cmdline.c_str());
		REQUIRE(!system(cmdline.c_str()));  // <<< Gives segmentation fault, can't find reason
		                                    //     Runs fine on command line

		std::string diff_cmd = "diff obj/test/" + output_filename + " " + expected_filename;
		REQUIRE(!system(diff_cmd.c_str()));
	}


	SECTION("Emulator should give correct output") {
		std::string output_filename   = "ReqRecv_output_emu.txt";
		std::string cmdline = BASE_CMDLINE;
		cmdline +=  "-r=emulator > " + output_filename;
		INFO("Cmdline: " << cmdline);
		REQUIRE(!system(cmdline.c_str()));  // <<< Idem

		std::string diff_cmd = "diff obj/test/" + output_filename + " " + expected_filename;
		REQUIRE(!system(diff_cmd.c_str()));
	}
}

