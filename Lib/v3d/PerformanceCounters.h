#ifndef _LIB_V3D_PERFORMANCECOUNTERS_H
#define _LIB_V3D_PERFORMANCECOUNTERS_H

#ifdef QPU_MODE

#include <vector>
#include <string>
#include <stdint.h>

namespace V3DLib {
namespace v3d {

/**
 * Performance counters for `v3d`
 *
 * ----------------------------------------------------------------------------
 * # Setup
 *
 * * There are an unknown number of counters (at least 33, at most 127) per core.
 *   Each core has its own set, the counters are the same per core.
 *
 * * In order to use a counter, it must be allocated to a 'source register'.
 *   There are 32 of these per core, and that's the maximum number of counters that you can
 *   access in one go.
 *
 * * To allocate a counter, a bit is set in register `CORE_PCTR_0_EN` to specify the source register.
 *   The counter to use in the selected source register is specified in the corresponding bit field
 *   in registers CORE_PCTR_0_SRC_0 to CORE_PCTR_0_SRC_7. Each of these registers contains four bitfields,
 *   for four consecutive source registers. The bitfields are 6 bits wide, this places an upper limit on
 *   the number of available counters.
 *
 * * Access to a counter is stopped by setting the corresponding source register bit in CORE_PCTR_0_EN to zero.
 *
 *
 * # Further notes
 *
 * * The v3d has one single core (`vc4` is organized differently and has no cores).
 *   This class thus takes one single core into account. Should more cores be added in future versions,
 *   the class needs to be adjusted.
 *
 * * The setup is comparable to the way the performance counters work on `vc4`. Differences are:
 *   - `v3d` has source registers per core, `vc4` a set of global registers
 *   - in `v3d` bitfields are used to specify counter per source register, `vc4` has no bitfields
 *
 *
 * # Source 
 *
 * Taken from `py-videocore6` project, which is all the informaction available on counters.
 * Source: https://github.com/Idein/py-videocore6/blob/58bbcb88979c8ee6c8bd847da884c2405994432b/videocore6/v3d.py#L241
 */
class PerformanceCounters {
public:
	enum {
		NUM_SRC_REGS = 32,

		//
		// Labels for counters
		//
		// Only one is known at this time, its value indicates that there
		// must be 32 preceding counters present.
		//
		CORE_PCTR_CYCLE_COUNT = 32,                    // Assumption: the number of clock cycles that a program ran.
		                                               // The number is variable per run, but always in the same range.

		NUM_PERF_COUNTERS = CORE_PCTR_CYCLE_COUNT + 1  // No idea how many there are, this is an assumption
	};

	static void enter(std::vector<int> srcs);
	static std::string showEnabled();

private:
	static const char *Description[NUM_PERF_COUNTERS];

	static void exit();
};


}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE

#endif  // _LIB_V3D_PERFORMANCECOUNTERS_H
