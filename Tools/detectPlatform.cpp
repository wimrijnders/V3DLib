#include <unistd.h>  // geteuid()
#include <string>
#include <fstream>
#include <streambuf>
#include <CmdParameters.h>
#include "QPULib.h"
#include "Support/Platform.h"
#include "vc4/vc4.h"
#include "vc4/Mailbox.h"
#include "vc4/RegisterMap.h"
#include "v3d/RegisterMapping.h"

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
 * @brief Collect and make general information available on the current platform
 *
 * In time, this struct will be made generic for all QPULib programs
 */
struct Settings {

	// cmdline param's
	bool reset_scheduler;


	/**
   * @brief Collect all the info we want
	 *
	 * @return ALL_IS_WELL if all is well and program can continue,
	 *         any other value if program should abort
   */
	int init(int argc, const char *argv[]) {
		auto platform = Platform::instance();

		platform.output();
		output();

		if (!platform.is_pi_platform) {
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
void detect_v3d() {
	v3d::RegisterMapping map_v3d;
	map_v3d.init();
	
	auto info = map_v3d.info();
	printf("Revision        : %d.%d.%d.%d\n", info.tver, info.rev, info.iprev, info.ipidx);
	printf("Number of cores : %d\n",   info.num_cores);
	printf("MMU             : %s\n", (info.mmu)?"yes":"no");
	printf("TFU             : %s\n", (info.tfu)?"yes":"no");
	printf("TSY             : %s\n", (info.tsy)?"yes":"no");
	printf("MSO             : %s\n", (info.mso)?"yes":"no");
	printf("L3C             : %s (%dkb)\n\n", (info.l3c)?"yes":"no", info.l3c_nkb);

	for (unsigned core = 0; core < info.num_cores; ++core) {
		auto info = map_v3d.info_per_core(core);

		printf("Core index %d:\n", info.index);
		printf("  Revision      : %d.%d\n", info.ver, info.rev);
		printf("  VPM size      : %d\n", info.vpm_size);
		printf("  Num slices    : %d\n", info.num_slice);
		printf("  Num TMU's     : %d (all slices)\n", info.num_tmu);
		printf("  Num QPU's     : %d (all slices)\n", info.num_qpu);
		printf("  Num semaphores: %d\n", info.num_semaphore);
		printf("  BCG int       : %s\n", (info.bcg_int)?"yes":"no");
		printf("  Override TMU  : %s\n", (info.override_tmu)?"yes":"no");
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

/*
	// DEBUG: read first three registers
	printf("Reg 0: %X\n", RegisterMap::readRegister(0));
	printf("Reg 1: %X\n", RegisterMap::readRegister(1));
	printf("Reg 2: %X\n", RegisterMap::readRegister(2));
*/

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
	if (Platform::instance().has_vc4) {
		detect_vc4();
	} else {
		detect_v3d();
	}
#endif  // QPU_MODE

	return 0;
}
