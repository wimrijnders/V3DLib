// 
// Converted from: https://github.com/Idein/py-videocore6/blob/ec275f668f8aa4c89839fb8095b74f402260b1a6/videocore6/driver.py
//
// Note 1:
//
// Setting workgroup value to non-zero has the effect that:
//
 //  - Some QPU's don't return anything, which QPU's varies
 //  - Other QPU's return 1 or a limited number of registers, which QPU's varies
 //  - The reg's that *are* returned, have the correct value
//
// special observed cases:
//
// (1,7,1)     0
// (1,1,1)     0
// (2,2,1)     0
//  - consistently 1 qpu with only reg 0 filled, rest filled
//
// 1,1,2     0
//  - consistently one QPU with first 4 re'gs filled, which QPU varies
//
// 1,1,1     1
//	- 4-6 QPU's return with only reg 0 filled, which QPU's vary
//
// 1,1,2     1
//	- 5-6 QPU's return with only reg 0 and 1 filled, which QPU's vary
//
//1,1,7     1
//	- >= 4 QPU's return with only reg's 0-6 filled, which QPU's vary
// 
// 1,7,2     3
// 0,7,2     3
//	- 3-5 QPU's return with only reg's 0-5 filled, which QPU's vary
//
///////////////////////////////////////////////////////////////////////////////
#include <algorithm>  // find()
#include "Support/debug.h"
#include "Driver.h"
#include "v3d.h"

namespace V3DLib {
namespace v3d {

/**
 * @return true if execution went well and no timeout,
 *         false otherwise
 *
 * ============================================================================
 * NOTES
 * =====
 *
 * * TODO use following for next step:
 *
 * https://github.com/Idein/py-videocore6/blob/master/benchmarks/test_gpu_clock.py
 */
bool Driver::execute(SharedArray<uint64_t> &code, SharedArray<uint32_t> *uniforms, uint32_t thread) {
	uint32_t code_phyaddr = code.getAddress();
	uint32_t unif_phyaddr = (uniforms == nullptr)?0u:uniforms->getAddress();

	assert(m_timeout_sec > 0);
	assertq(m_bo_handles.size() > 0, "v3d execute: There should be at least one buffer object present on execution");

	// See Note 1
	WorkGroup workgroup = (1, 1, 0);  // Setting last val to 0 ensures all QPU's return all registers
	uint32_t wgs_per_sg = 16;         // Has no effect if previous (1, 1, 0)

	st_v3d_submit_csd st = {
		{
			workgroup.wg_x << 16,
			workgroup.wg_y << 16,
			workgroup.wg_z << 16,
			(
        ((((wgs_per_sg * workgroup.wg_size() + 16u - 1u) / 16u) - 1u) << 12) |
				(wgs_per_sg << 8) |
				workgroup.wg_size() & 0xff
			),
			thread - 1,           // Number of batches minus 1
			code_phyaddr,         // Shader address, pnan, singleseg, threading
			unif_phyaddr
		},
		{0,0,0,0},
		(uint64_t) m_bo_handles.data(),
		m_bo_handles.size(),
		0,   // in_sync
		0    // out_sync
	};

	uint64_t timeout_ns = 1000000000llu * m_timeout_sec;

	bool ret = (0 == v3d_submit_csd(st));
	assert(ret);
	if (ret) {
		ret = v3d_wait_bo(m_bo_handles, timeout_ns);
	}
	return ret;
}

}  // v3d
}  // V3DLib
