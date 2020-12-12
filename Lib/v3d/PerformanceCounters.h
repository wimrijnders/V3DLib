#ifndef _LIB_V3D_PERFORMANCECOUNTERS_H
#define _LIB_V3D_PERFORMANCECOUNTERS_H

#ifdef QPU_MODE

#include <vector>
#include <stdint.h>

namespace V3DLib {
namespace v3d {

/**
 * Taken from `py-videocore6` project, which is all the informaction available on counters.
 * Source: https://github.com/Idein/py-videocore6/blob/58bbcb88979c8ee6c8bd847da884c2405994432b/videocore6/v3d.py#L241
 */
class PerformanceCounters {
private:
	enum {
		NUM_SRC_REGS = 8,
		CORE_PCTR_CYCLE_COUNT = 32
	};

public:
	PerformanceCounters();

	void enter();
	void exit();
	void result();

private:
	int core_id = 0;  // Asumming 1 core with id == 0 sufficient for now
	std::vector<int> m_srcs;
	uint32_t m_mask = 0;;
};


}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE

#endif  // _LIB_V3D_PERFORMANCECOUNTERS_H
