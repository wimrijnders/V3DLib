#include "catch.hpp"
#include <unistd.h>           // for geteuid()
#include <sys/types.h>        // idem
#include "Support/Platform.h"
#include "support/support.h"  // running_on_v3d()


//
// get the base directory right for calling compiled apps.
//
#ifdef DEBUG
  #define POSTFIX_DEBUG "-debug"
#else
  #define POSTFIX_DEBUG ""
#endif

#ifdef QPU_MODE
//  #pragma message "QPU mode enabled"
#define POSTFIX_QPU "qpu"
#else
#define POSTFIX_QPU "emu"
#endif

#define BIN_PATH "obj/" POSTFIX_QPU POSTFIX_DEBUG "/bin"

namespace {

enum RunType {
  QPU,
  EMULATOR,
  INTERPRETER
};


void init_msg() {
  if (!V3DLib::Platform::has_vc4()) {
    static bool showed_msg = false;
    if (showed_msg) return;

    if (geteuid() != 0) {
      printf("NOTE: test [cmdline] will only work with [v3d][code] and [v3d][driver] "
             "if `runTest` is run with `sudo`.\n");

      showed_msg = true;
    }
  }
}


void check_output_run(std::string const &program, RunType run_type, std::string const &extra_params) {
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

  if (!extra_params.empty()) {
    params += " ";
    params += extra_params;
    params += " ";
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


void check_output_example(std::string const &program, std::string const &extra_params = "") {
  check_output_run(program, QPU, extra_params);
  check_output_run(program, INTERPRETER, extra_params);
  check_output_run(program, EMULATOR, extra_params);
}

}  // anon namespace


//
// This test is fairly useless; it just checks if both the shell script and the C++ program
// fail or succeed at the same time.
//
// Better would be to check if the first line is the same
//
TEST_CASE("Detect platform scripts should both return the same thing", "[cmdline]") {
  init_msg();
  int ret1 = system(BIN_PATH "/detectPlatform > /dev/null");
  bool success1 = (ret1 == 0);

  int ret2 = system("Tools/detectPlatform.sh > /dev/null");
  bool success2 = (ret2 == 0);

  INFO("C++ script returned " << ret1 << ", shell script returned " << ret2);
  REQUIRE(success1 == success2);
}


TEST_CASE("Check correct output example programs for all three run options", "[cmdline]") {
  init_msg();
  make_test_dir();

  SECTION("Check output ReqRecv") { check_output_example("ReqRecv"); }
  SECTION("Check output ID")      { check_output_example("ID"); }
  SECTION("Check output Hello")   { check_output_example("Hello"); }
  SECTION("Check output GCD")     { check_output_example("GCD"); }
  SECTION("Check output Tri")     { check_output_example("Tri"); }
  SECTION("Check output OET")     { check_output_example("OET"); }

  // Rot3D, the expected output is taken from the scalar kernel
  // For v3d, the match should be exact in all cases.
  //
  // For vc4, there is difference in output due to rounding.
  // In addition, at time of writing the multi-QPU version of kernel 2 is not working

  if (running_on_v3d()) {
    SECTION("Check output Rot3D")   { check_output_example("Rot3D", "-d -k=2"); }
    SECTION("Check output Rot3D")   { check_output_example("Rot3D", "-d -k=1"); }
  } else {
    // These should be no problem
    check_output_run("Rot3D", INTERPRETER, "-d -k=1");
    check_output_run("Rot3D", EMULATOR,    "-d -k=1");
    check_output_run("Rot3D", INTERPRETER, "-d -k=2");
    check_output_run("Rot3D", EMULATOR,    "-d -k=2");

    // Running on QPU will fail due to rounding errors
    // This gets checked in `testRot3D`, where the kernels are run directly
    //check_output_run("Rot3D", QPU, "-d -k=1");
  }
}
