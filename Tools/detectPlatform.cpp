#include <unistd.h>  // geteuid()
#include <string.h>  // strstr()
#include <string>
#include <fstream>
#include <streambuf>
#include <CmdParameters.h>
#include "QPULib.h"
#include "VideoCore/VideoCore.h"
#include "VideoCore/RegisterMap.h"
#include "VideoCore/vc6/RegisterMapping.h"

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
 * @param content - output; store platform string here if found
 *
 * @return true if string describing platform found,
 *         false otherwise.
 */
bool get_platform_string(std::string &content) {
	const char *filename = "/sys/firmware/devicetree/base/model";

	bool success = loadFileInString(filename, content);
	if (!success) {
		content == "";
	}

	return success;
}


/**
 * @brief Retrieve the VideoCore chip number.
 *
 * This is the way to detect the Pi platform for older Pi versions/distributions.
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

bool get_chip_version(std::string &output) {
	const char *BCM_VERSION_PREFIX = "BCM2";
	const char *filename = "/proc/cpuinfo";

	output.clear();

	std::ifstream t(filename);
	if (!t.is_open()) return false;

	std::string line;
	while (getline(t, line)) {
	  if (!strstr(line.c_str(), "Hardware")) continue;

		if (strstr(line.c_str(), BCM_VERSION_PREFIX)) {
	  	// For now, don't try to exactly specify the model.
			// This could be done with field "Revision' in current input.

			size_t pos = line.find(BCM_VERSION_PREFIX);
			if (pos != line.npos) {
				output = line.substr(pos);
			}

			return true;
		}
  }

	return false;
}


/**
 * @brief Collect and make general information available on the current platform
 *
 * In time, this struct will be made generic for all QPULib programs
 */
struct Settings {
	std::string platform_id; 
	std::string chip_version;
	bool is_pi_platform;
	bool has_vc4 = false;


	// cmdline param's
	bool reset_scheduler;


	/**
   * @brief Collect all the info we want
	 *
	 * @return ALL_IS_WELL if all is well and program can continue,
	 *         any other value if program should abort
   */
	int init(int argc, const char *argv[]) {
		is_pi_platform = get_platform_string(platform_id);
		if (get_chip_version(chip_version)) {
			is_pi_platform = true;
		}

		if (!platform_id.empty() && is_pi_platform) {
			has_vc4 = (platform_id.npos == platform_id.find("Pi 4"));
		}
		output();

		if (!is_pi_platform) {
			return CmdParameters::EXIT_ERROR;
		}

#ifndef QPU_MODE
		printf("Note: QPU mode is not enabled for this build. To enable, recompile with QPU=1 defined.\n\n");
		return CmdParameters::EXIT_NO_ERROR;
#else
		if (geteuid() != 0) {  // Only do this as root (sudo)
			printf("You need to run this with `sudo` to access the device file\n\n");
			return CmdParameters::EXIT_NO_ERROR;
		}
#endif  // QPU_MODE

		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		reset_scheduler = params.parameters()[0]->get_bool_value();

		return CmdParameters::ALL_IS_WELL;
	}


	void output() {
		if (!platform_id.empty()) {
			printf("Platform: %s\n", platform_id.c_str());
		} else {
			printf("Platform: %s\n", "Unknown");
		}

		printf("Chip version: %s\n", chip_version.c_str());

		if (!is_pi_platform) {
			printf("This is NOT a pi platform!\n");
		} else {
			printf("This is a pi platform.\n");

			if (has_vc4) {
				printf("GPU: vc4\n");
			} else {
				printf("GPU: vc6\n");
			}
		}

		printf("\nCmdline param's:\n");
		printf("  Reset Scheduler  : %s\n", reset_scheduler?"true":"false");
		printf("\n");
	}
	
} settings;


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


/**
 * This should compare with:
 *
 *     > sudo cat /sys/kernel/debug/dri/0/v3d_ident
 *     Revision:   4.2.14.0
 *     MMU:        yes
 *     TFU:        yes
 *     TSY:        yes
 *     MSO:        yes
 *     L3C:        no (0kb)
 *     Core 0:
 *       Revision:     4.2
 *       Slices:       2
 *       TMUs:         2
 *       QPUs:         8
 *       Semaphores:   0
 *       BCG int:      0
 *       Override TMU: 0
 *      
 *
 * However, at time of writing, while testing this method, this generated multiple
 * errors:
 *
 *     ....quite a few more before this....
 *     Message from syslogd@pi4-3 at Jun  1 12:10:59 ...
 *     kernel:[69733.669058] 1fe0: 0000006c be858550 0001438c b6e9b880 60000010 00000003 00000000 00000000
 *    
 *    Message from syslogd@pi4-3 at Jun  1 12:10:59 ...
 *     kernel:[69733.669496] Code: e5933000 e593300c e5933018 e5933014 (e5933008) 
 *    Segmentation fault
 */
void detect_vc6() {
	vc6::RegisterMapping map_vc6;
	map_vc6.init();
	
	unsigned ncores = map_vc6.num_cores();
	printf("Number of cores    : %d\n",   ncores);

	for (unsigned core = 0; core < ncores; ++core) {
		auto info = map_vc6.core_info(core);

		printf("Core index      : %d\n",   info.index);
		printf("VPM size        : %d\n",   info.vpm_size);
		printf("Num slices      : %d\n",   info.num_slice);
		printf("Num TMU's       : %d\n",   info.num_tmu);
		printf("Num QPU's       : %d\n",   info.num_qpu);
	}
}


void detect_vc4() {
	enableQPUs();

	if (settings.reset_scheduler) {
		RegisterMap::resetAllSchedulerRegisters();
	}

	int mb = getMailbox();	
	unsigned revision = get_version(mb);
	printf("Hardware revision        : %04x\n", revision);

	printf("Tech version             : %d\n", RegisterMap::TechnologyVersion());
	printf("Number of slices         : %d\n", RegisterMap::numSlices());
	printf("Number of QPU's per slice: %d\n", RegisterMap::numQPUPerSlice());
	printf("Number of TMU's per slice: %d\n", RegisterMap::numTMUPerSlice());
	printf("VPM memory size (KB)     : %d\n", RegisterMap::VPMMemorySize());
	printf("L2 Cache enabled         : %s\n", (RegisterMap::L2CacheEnabled())? "yes": "no");
	showSchedulerRegisters();
	printf("\n");

	disableQPUs();
}

#endif  // QPU_MODE


/**
 * @brief Detect if this is running on a Rpi.
 *
 * @returns 0 if this is so, 1 if it's a different platform.
 */
int main(int argc, char const *argv[]) {
	int ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

#ifdef  QPU_MODE
	if (settings.has_vc4) {
		detect_vc4();
	} else {
		detect_vc6();
	}
#endif  // QPU_MODE

	return 0;
}
