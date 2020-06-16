#include <unistd.h>  // geteuid()
#include <string.h>  // strstr()
#include <string>
#include <fstream>
#include <streambuf>
#include <CmdParameters.h>
#include "QPULib.h"
#include "VideoCore/VideoCore.h"
#include "VideoCore/RegisterMap.h"

using namespace QPULib;

CmdParameters params = {
	"Show info on the platform and the VideoCore.\n\n"
	"This mostly reads the VideoCore registers to retrieve information about the device\n"
	"There is limited possibility to manipulate these registers.",
	{{
		"Reset Scheduler Registers",
		"-r", // "--reset-scheduler",
		ParamType::NONE,
		"Clear the prohibition bits in the scheduler registers that determine what kind"
		"of program can run."
	}}
};


struct Settings {
	bool reset_scheduler;

	int init(int argc, const char *argv[]) {
		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		reset_scheduler = params.parameters()[0]->get_bool_value();
		output();

		return ret;
	}

	void output() {
		printf("Settings:\n");
		printf("  Reset Scheduler  : %s\n", reset_scheduler?"true":"false");
		printf("\n");
	}
} settings;


/**
 * @brief read the entire contents of a file into a string
 *
 * @param filename name of file to read
 * @param out_str  output parameter; place to store the file contents
 *
 * @return true if all went well, false if file could not be read.
 */
bool loadFileInString(const char *filename, std::string & out_str) {
	std::ifstream t(filename);
	if (!t.is_open()) {
		return false;
	}

	std::string str((std::istreambuf_iterator<char>(t)),
  	               std::istreambuf_iterator<char>());

	if (str.empty()) {
		return false;
	}

	out_str = str;
	return true;
}


/**
 * @brief Detect Pi platform for newer Pi versions.
 *
 * On success, it displays a string with the model version.
 *
 * @return true if Pi detected, false otherwise
 */
bool detect_from_sys() {
	const char *filename = "/sys/firmware/devicetree/base/model";

	std::string content;
	bool success = loadFileInString(filename, content);
	if (success && !content.empty()) {
		printf("Detected platform: %s\n", content.c_str());
    return true;
	}

	return false;
}


/**
 * @brief Detect Pi platform for older Pi versions.
 *
 * Detects if this is a VideoCore. This should be sufficient for detecting Pi,
 * since it's the only thing to date(!) using this particular chip version.
 *
 * @return true if Pi detected, false otherwise
 *
 * --------------------------------------------------------------------------
 * ## NOTES
 *
 * * The following are valid model numbers:
 *
 *  - BCM2708
 *  - BCM2835    - This appears to be returned for all higher BCM versions
 *
 * * The following are also valid, but appear to be represented by 'BCM2835'
 *   in `/proc/cpuinfo`:
 *
 *  - BCM2836   // If that's not the case, enable these as well
 *  - BCM2837
 *  - BCM2837B0
 */
bool detect_from_proc() {
	const char *BCM_VERSION_PREFIX = "BCM2";
	const char *filename = "/proc/cpuinfo";

	std::ifstream t(filename);
	if (!t.is_open()) return false;

	std::string line;
	while (getline(t, line)) {
	  if (!strstr(line.c_str(), "Hardware")) continue;

		if (strstr(line.c_str(), BCM_VERSION_PREFIX)) {
	  	// For now, don't try to exactly specify the model.
			// This could be done with field "Revision' in current input.
			printf("This is a Pi platform\n");
			return true;
		}
  }

	return false;
}


#ifdef QPU_MODE
void showSchedulerRegisters() {
	int numQPUs = RegisterMap::numSlices() * RegisterMap::numQPUPerSlice();
	SchedulerRegisterValues values = RegisterMap::SchedulerRegisters();

	printf("Scheduler registers, do not use:");

	for (int i = 0; i < numQPUs; ++i ) {
		printf("\n  QPU %02d: ", i);

		int val = values.qpu[i];

		if (val & DO_NOT_USE_FOR_USER_PROGRAMS) {
			printf("user programs, ");
		}

		if (val & DO_NOT_USE_FOR_FRAGMENT_SHADERS) {
			printf("fragment shaders, ");
		}

		if (val & DO_NOT_USE_FOR_VERTEX_SHADERS) {
			printf("vertex shaders, ");
		}

		if (val & DO_NOT_USE_FOR_COORDINATE) {
			printf("coordinate");
		}
	}

	printf("\n");
}
#endif  // QPU_MODE


/**
 * @brief Detect if this is running on a Rpi.
 *
 * @returns 0 if this is so, 1 if it's a different platform.
 */
int main(int argc, char const *argv[]) {
	if (!detect_from_sys() && !detect_from_proc()) {
		printf("This is not a Pi platform\n");
		return 1;
	}

	printf("\n");

#ifndef QPU_MODE
	printf("QPU code is not enabled for this build. To enable, recompile with QPU=1 defined.\n\n");
	return 1;
#else
	if (geteuid() != 0) {  // Only do this as root (sudo)
		printf("You need to run this with `sudo` to access the device file\n\n");
		return 1;
	}

	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

	if (settings.reset_scheduler) {
		RegisterMap::resetAllSchedulerRegisters();
	}

	enableQPUs();

	int mb = getMailbox();	
	unsigned revision = get_version(mb);
	printf("Hardware revision        : %04x\n", revision);

	printf("Tech version             : %d\n", RegisterMap::TechnologyVersion());
	printf("Number of slices         : %d\n",   RegisterMap::numSlices());
	printf("Number of QPU's per slice: %d\n",   RegisterMap::numQPUPerSlice());
	printf("Number of TMU's per slice: %d\n",   RegisterMap::numTMUPerSlice());
	printf("VPM memory size (KB)     : %d\n",   RegisterMap::VPMMemorySize());
	showSchedulerRegisters();
	printf("\n");

	disableQPUs();
#endif

	return 0;
}
