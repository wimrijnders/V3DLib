//
// Basic command line handling for the example programs.
//
// This contains common functionality for all examples.
//
///////////////////////////////////////////////////////////////////////////////
#include "Settings.h"
#include <memory>
#include <CmdParameters.h>


namespace {

const char *blurb =
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
;


std::unique_ptr<CmdParameters> params;

CmdParameters &instance() {
	if (!params) {
		params.reset( new CmdParameters{
			blurb,
			{{
				"Output Generated Code",
				"-f",
				ParamType::NONE,     // Prefix needed to dsambiguate
				"Write representations of the generated code to file"
			}, { 
				"Compile Only",
				"-c",
				ParamType::NONE,     // Prefix needed to dsambiguate
				"Compile the kernel but do not run it"
			}, {
				"Select run type",
				"-r=",
				{"default", "emulator", "interpreter"},
				"Run on the QPU emulator or run on the interpreter"
			}}
		});
	}

	return *params;
}


/**
 * Get the base filename, without extension from a path
 *
 * Source: https://stackoverflow.com/a/8520815
 *
 * In C++17, you can use `stem()`: https://en.cppreference.com/w/cpp/filesystem/path/stem
 */
std::string stem(const char *input) {
	std::string filename(input);

	// Remove directory if present.
	// Do this before extension removal incase directory has a period character.
	const size_t last_slash_idx = filename.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		filename.erase(0, last_slash_idx + 1);
	}

	// Remove extension if present.
	const size_t period_idx = filename.rfind('.');
	if (std::string::npos != period_idx)
	{
		filename.erase(period_idx);
	}

	return filename;
}

}  // anon namespace


namespace QPULib {


int Settings::init(int argc, const char *argv[]) {
	name = stem(argv[0]);

	auto ret = instance().handle_commandline(argc, argv, false);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

	process();

	return ret;
}


void Settings::process(CmdParameters *in_params) {
	CmdParameters &params = (in_params != nullptr)?*in_params:instance();

	output_code  = params.parameters()["Output Generated Code"]->get_bool_value();
	compile_only = params.parameters()["Compile Only"]->get_bool_value();
	run_type     = params.parameters()["Select run type"]->get_int_value();
}


CmdParameters &Settings::params() {
	return instance();
}

}  // namespace QPULib;
