//
// Basic command line handling for the example programs.
//
// This contains common functionality for all examples.
//
///////////////////////////////////////////////////////////////////////////////
#include "Settings.h"

namespace {

CmdParameters params = {
	"Example Program\n",			// TODO Perhaps fill this in dynamically
	{{
		"Output Generated Code",
		"-c",
		ParamType::NONE,     // Prefix needed to dsambiguate
		"Write representations of the generated code to file"
	}}
};

}  // anon namespace


namespace QPULib {


int Settings::init(int argc, const char *argv[]) {
	auto ret = params.handle_commandline(argc, argv, false);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

/*
	kernel      = params.parameters()[0]->get_int_value();
	kernel_name = params.parameters()[0]->get_string_value();
*/
	output_code  = params.parameters()[0]->get_bool_value();

#ifdef DEBUG
	output();
#endif

	return ret;
}


void Settings::output() {
	printf("Settings:\n");
	printf("  Output Code : %s\n", output_code?"true":"false");
	printf("\n");
}

}  // namespace QPULib;
