#ifndef _VC6_DRIVER_H_
#define _VC6_DRIVER_H_
#include "drm_types.h"
#include "DRM_V3D.h"


namespace QPULib {
namespace vc6 {

class Dispatcher {
public:
   Dispatcher(DRM_V3D &drm, BoHandles bo_handles, int timeout_sec = 10);

    void exit(); 

    void dispatch(
			Code &code,
			Uniforms *uniforms = nullptr,
			WorkGroup workgroup = WorkGroup(),
			uint32_t wgs_per_sg = 16,
			uint32_t thread = 1);

private:
	DRM_V3D &m_drm;
	BoHandles m_bo_handles = nullptr;
  int m_timeout_sec = -1;
};


class Driver {
private:
	void v3d_submit_csd(
		Cfg cfg,
		Uniforms *uniforms,
		Coef coef,
		BoHandles bo_handles,
		int bo_handle_count,
		int in_sync,
		int out_sync);
	

	DRM_V3D m_drm;
	BoHandles m_bo_handles = nullptr;

	void v3d_wait_bo(uint32_t bo_handle, int timeout);
	Dispatcher compute_shader_dispatcher(int timeout_sec= 10);

	void execute(
		Code &code,
		Uniforms *uniforms,
		int timeout_sec = 10,
		WorkGroup workgroup = (16, 1, 1),
		int wgs_per_sg = 16,
		int thread = 1);

};  // class Driver

}  // vc6
}  // QPULib

#endif // _VC6_DRIVER_H_
