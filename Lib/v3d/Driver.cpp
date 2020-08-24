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
#include <cassert>
#include <algorithm>  // find()
#include "Driver.h"
#include "v3d.h"

namespace QPULib {
namespace v3d {


///////////////////////////////////////////////////////////////////////////////
// Class Driver
///////////////////////////////////////////////////////////////////////////////

/**
* TODO: use following for next step:
*
* https://github.com/Idein/py-videocore6/blob/master/benchmarks/test_gpu_clock.py
*/
bool Driver::dispatch(
	uint32_t code_phyaddr,
	uint32_t unif_phyaddr,
	uint32_t thread
) {
	assert(m_timeout_sec > 0);
	assert(m_bo_handles.size() > 0);  // There should be at least one, for the code

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
		0,
		0
	};

	uint64_t  timeout_ns = 1000000000llu * m_timeout_sec;

	int ret = v3d_submit_csd(st);
	assert(ret == 0);
	v3d_wait_bo(m_bo_handles, timeout_ns);

	return ret == 0;
}


bool Driver::execute(
	SharedArray<uint64_t> &code,
	SharedArray<uint32_t> *uniforms,
	int thread) {

	return dispatch(
		code.getAddress(),
		((uniforms == nullptr)?0u:uniforms->getAddress()),
		thread);
}

}  // v3d
}  // QPULib
