//
// Basic command line handling for the example programs.
//
// This contains common functionality for all examples.
//
///////////////////////////////////////////////////////////////////////////////
#include "Settings.h"
#include <memory>
#ifdef QPU_MODE
#include "Support/Platform.h"
#include "vc4/PerformanceCounters.h"
#endif  // QPU_MODE

#ifdef QPU_MODE
using PC = QPULib::PerformanceCounters;
#endif  // QPU_MODE

namespace {

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


// ============================================================================
// Performance Counters 
// ============================================================================

#ifdef QPU_MODE
/**
 * @brief Enable the counters we are interested in
 */
void initPerfCounters() {
	PC::Init list[] = {
		{ 0, PC::QPU_INSTRUCTIONS },
		{ 1, PC::QPU_STALLED_TMU },
		{ 2, PC::L2C_CACHE_HITS },
		{ 3, PC::L2C_CACHE_MISSES },
		{ 4, PC::QPU_INSTRUCTION_CACHE_HITS },
		{ 5, PC::QPU_INSTRUCTION_CACHE_MISSES },
		{ 6, PC::QPU_CACHE_HITS },
		{ 7, PC::QPU_CACHE_MISSES },
		{ 8, PC::QPU_IDLE },
		{ PC::END_MARKER, PC::END_MARKER }
	};

	PC::enable(list);
	PC::clear(PC::enabled());

	//printf("Perf Count mask: %0X\n", PC::enabled());

	// The following will show zeroes for all counters, *except*
	// for QPU_IDLE, because this was running from the clear statement.
	// Perhaps there are more counters like that.
	//std::string output = PC::showEnabled();
	//printf("%s\n", output.c_str());
}
#endif  // QPU_MODE


// ============================================================================
// Settings 
// ============================================================================

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
#ifdef QPU_MODE
			}, {
		    "Performance Counters",
		    "-pc",
				ParamType::NONE,
		    "Show the values of the performance counters (vc4 only)"
#endif  // QPU_MODE
			}}
		});
	}

	return *params;
}

}  // anon namespace


namespace QPULib {


int Settings::init(int argc, const char *argv[]) {
	set_name(argv[0]);

	auto ret = instance().handle_commandline(argc, argv, false);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

	process();

	return ret;
}


void Settings::set_name(const char *in_name) {
	assert(in_name != nullptr);
	name = stem(in_name);
}


void Settings::process(CmdParameters *in_params) {
	CmdParameters &params = (in_params != nullptr)?*in_params:instance();

	output_code  = params.parameters()["Output Generated Code"]->get_bool_value();
	compile_only = params.parameters()["Compile Only"]->get_bool_value();
	run_type     = params.parameters()["Select run type"]->get_int_value();
#ifdef QPU_MODE
	show_perf_counters  = params.parameters()["Performance Counters"]->get_bool_value();
#endif  // QPU_MODE
}


void Settings::startPerfCounters() {
	//printf("Entered Settings::startPerfCounters()\n");

#ifdef QPU_MODE
	if (show_perf_counters) {
		if (Platform::instance().has_vc4) {  // vc4 only
			initPerfCounters();
		} else {
			printf("WARNING: Performance counters enabled for VC4 only.\n");
		}
	}
#endif
}


void Settings::stopPerfCounters() {
#ifdef QPU_MODE
	if (show_perf_counters) {
		if (Platform::instance().has_vc4) {  // vc4 only
			// Show values current counters
			std::string output = PC::showEnabled();
			printf("%s\n", output.c_str());
		}
	}
#endif
}


CmdParameters &Settings::params() {
	return instance();
}

}  // namespace QPULib;
