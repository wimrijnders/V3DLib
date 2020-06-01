#include <unistd.h>  // geteuid()
#include <string.h>  // strstr()
#include <string>
#include <fstream>
#include <streambuf>
#include "QPULib.h"
#include "VideoCore/VideoCore.h"
#include "VideoCore/RegisterMap.h"
#include "VideoCore/vc6/RegisterMapping.h"

using namespace QPULib;


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


void detect_vc4() {
	printf("Tech version             : %d\n", RegisterMap::TechnologyVersion());
	printf("Number of slices         : %d\n",   RegisterMap::numSlices());
	printf("Number of QPU's per slice: %d\n",   RegisterMap::numQPUPerSlice());
	printf("Number of TMU's per slice: %d\n",   RegisterMap::numTMUPerSlice());
	printf("VPM memory size (KB)     : %d\n",   RegisterMap::VPMMemorySize());
	showSchedulerRegisters();
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

#endif  // QPU_MODE


/**
 * @brief Detect if this is running on a Rpi.
 *
 * @returns 0 if this is so, 1 if it's a different platform.
 */
int main(int argc, char *argv[]) {
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

	bool vc6 = true;

	if (vc6) {
		detect_vc6();
	} else {

	enableQPUs();

	int mb = getMailbox();	
	unsigned revision = get_version(mb);
	printf("Hardware revision        : %04x\n", revision);

	detect_vc4();

	disableQPUs();

	}  // vc6
#endif

	return 0;
}
