//
// Basic command line handling for the example programs.
//
// This contains common functionality for all examples.
//
///////////////////////////////////////////////////////////////////////////////
#include "Settings.h"

namespace {

CmdParameters params = {
	"Example Program\n"			// TODO Perhaps fill this in dynamically
#ifdef EMULATION_MODE
	"\nRunning in emulation mode.\n"
#ifdef QPU_MODE
	"\nRunning in QPU mode AND  emulation mode! This will likely lead to segmentation faults.\n"
#endif
#endif
#ifdef QPU_MODE
	"\nRunning in QPU mode.\n"
#endif
	,
	{{
		"Output Generated Code",
		"-c",
		ParamType::NONE,     // Prefix needed to dsambiguate
		"Write representations of the generated code to file"
	}
#ifdef EMULATION_MODE
	, {
		"Select run type",
		"-r=",
		{"default", "emulator", "interpreter"},
		"Run on the QPU emulator or run on the emulator"
	}
#endif
	}
};

}  // anon namespace


namespace QPULib {


int Settings::init(int argc, const char *argv[]) {
	auto ret = params.handle_commandline(argc, argv, false);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

/*
	kernel_name = params.parameters()[0]->get_string_value();
*/
	output_code  = params.parameters()[0]->get_bool_value();
#ifdef EMULATION_MODE
	run_type     = params.parameters()[1]->get_int_value();
#endif

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
