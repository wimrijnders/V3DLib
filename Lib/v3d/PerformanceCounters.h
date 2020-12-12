#ifndef _LIB_V3D_PERFORMANCECOUNTERS_H
#define _LIB_V3D_PERFORMANCECOUNTERS_H

#ifdef QPU_MODE

#include <vector>
#include <string>
#include <stdint.h>

namespace V3DLib {
namespace v3d {

/**
 * Taken from `py-videocore6` project, which is all the informaction available on counters.
 * Source: https://github.com/Idein/py-videocore6/blob/58bbcb88979c8ee6c8bd847da884c2405994432b/videocore6/v3d.py#L241
 *
 * TODO: align with vc4 perf counters
 */
class PerformanceCounters {
public:
	enum {
		NUM_SRC_REGS = 8,
		CORE_PCTR_CYCLE_COUNT = 32,

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
