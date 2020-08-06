// 
// Converted from: https://github.com/Idein/py-videocore6/blob/ec275f668f8aa4c89839fb8095b74f402260b1a6/videocore6/driver.py
//
///////////////////////////////////////////////////////////////////////////////
#include <cassert>
#include "Driver.h"

namespace QPULib {
namespace v3d {


///////////////////////////////////////////////////////////////////////////////
// Class Dispatcher
///////////////////////////////////////////////////////////////////////////////

Dispatcher::Dispatcher(
	DRM_V3D &drm,
	BoHandles &bo_handles,
	int timeout_sec) :
	m_drm(drm),
	m_bo_handles(bo_handles),
	m_timeout_sec(timeout_sec) {}


Dispatcher::~Dispatcher() {
	for (int index = 0; index < m_bo_handles.size(); ++index) {
		auto bo_handle = m_bo_handles[index];
		m_drm.v3d_wait_bo(bo_handle, (uint64_t) (m_timeout_sec / 1e-9));
	}
}


/**
* TODO: use following for next step:
*
* https://github.com/Idein/py-videocore6/blob/master/benchmarks/test_gpu_clock.py
*/
void Dispatcher::dispatch(
	Code &code,
	Array &uniforms,
	WorkGroup workgroup,
	uint32_t wgs_per_sg,
	uint32_t thread
) {
	// NOTE: default is		workgroup = {16, 1, 1};

	auto roundup = [] (uint32_t n, uint32_t d) -> int {
		assert(n+d > 0);
		return (n + d - 1); // d
	};
			

	Cfg cfg = {
		// WGS X, Y, Z and settings
		workgroup.wg_x << 16,
		workgroup.wg_y << 16,
		workgroup.wg_z << 16,
		(
			(roundup(wgs_per_sg * workgroup.wg_size(), 16) - 1) << 12) |
			(wgs_per_sg << 8) |
			(workgroup.wg_size() & 0xff
		),
		thread - 1,           // Number of batches minus 1
		code.getPhyAddr(),    // Shader address, pnan, singleseg, threading
		uniforms.getPhyAddr(),
		//(uint32_t) (uniforms.size() > 0)?uniforms.getAddress(): 0
	};

	Coef coef = {0,0,0,0};

	m_drm.v3d_submit_csd(
		cfg,
		coef,
		m_bo_handles,
		0,
		0
	);
}


///////////////////////////////////////////////////////////////////////////////
// Class Driver
///////////////////////////////////////////////////////////////////////////////


Dispatcher Driver::compute_shader_dispatcher(int timeout_sec) {
	return Dispatcher(m_drm, m_bo_handles, timeout_sec);
}


void Driver::execute(
	Code &code,
	Array &uniforms,
	int thread,
	int timeout_sec,
	WorkGroup workgroup,
	int wgs_per_sg) {

	auto csd = compute_shader_dispatcher(timeout_sec);
  csd.dispatch(code, uniforms, workgroup, wgs_per_sg, thread);
}

}  // v3d
}  // QPULib
