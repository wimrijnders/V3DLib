#ifndef _VC6_DRIVER_H_
#define _VC6_DRIVER_H_
#include "drm_types.h"
#include "DRM_V3D.h"


namespace QPULib {
namespace v3d {

class Dispatcher {
  using SharedArray = Target::SharedArray<uint32_t>;

public:
	Dispatcher(
		DRM_V3D &drm,
		BoHandles bo_handles,
		uint32_t bo_handle_count,
		int timeout_sec);

	~Dispatcher(); 

	void dispatch(
		SharedArray &code,
		SharedArray *uniforms = nullptr,
		WorkGroup workgroup = WorkGroup(),
		uint32_t wgs_per_sg = 16,
		uint32_t thread = 1);

private:
	DRM_V3D &m_drm;
	BoHandles m_bo_handles = 0;
	uint32_t m_bo_handle_count = 0;
  int m_timeout_sec = -1;
};


class Driver {
  using SharedArray = Target::SharedArray<uint32_t>;

public:
	Dispatcher compute_shader_dispatcher(int timeout_sec= 10);

private:
	DRM_V3D m_drm;
	BoHandles m_bo_handles = nullptr;
	uint32_t m_bo_handle_count = 0;

	void execute(
		SharedArray &code,
		SharedArray *uniforms,
		int timeout_sec = 10,
		WorkGroup workgroup = (16, 1, 1),
		int wgs_per_sg = 16,
		int thread = 1);

};  // class Driver

}  // v3d
}  // QPULib

#endif // _VC6_DRIVER_H_
