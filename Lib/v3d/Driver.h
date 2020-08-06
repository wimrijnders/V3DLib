#ifndef _VC6_DRIVER_H_
#define _VC6_DRIVER_H_
#include "v3d/SharedArray.h"
#include "DRM_V3D.h"


namespace QPULib {
namespace v3d {

class Dispatcher {
  using Code  = ISharedArray; //<uint64_t>;
  using Array = ISharedArray;  //<uint32_t>;

public:
	Dispatcher(
		DRM_V3D &drm,
		BoHandles &bo_handles,
		int timeout_sec);

	~Dispatcher(); 

	void dispatch(
		Code &code,
		Array *uniforms = nullptr,
		WorkGroup workgroup = WorkGroup(),
		uint32_t wgs_per_sg = 16,
		uint32_t thread = 1);

private:
	DRM_V3D   &m_drm;
	BoHandles  m_bo_handles;
  int        m_timeout_sec = -1;
};


/**
 * NOTE: In python call, following was done in ctor:
 *
 *         self.bo_handles = np.array([self.memory.handle], dtype=np.uint32)
 *
 * TODO: check if this is relevant
 */
class Driver {
  using Code  = ISharedArray; //<uint64_t>;
  using Array = ISharedArray;  //<uint32_t>;

public:
	Dispatcher compute_shader_dispatcher(int timeout_sec= 10);

	void execute(
		Code &code,
		Array *uniforms,
		int thread = 1,
		int timeout_sec = 10,
		WorkGroup workgroup = (16, 1, 1),
		int wgs_per_sg = 16);

private:
	DRM_V3D m_drm;
	BoHandles m_bo_handles;

};  // class Driver

}  // v3d
}  // QPULib

#endif // _VC6_DRIVER_H_
