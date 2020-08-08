#ifndef _VC6_DRIVER_H_
#define _VC6_DRIVER_H_
#include "v3d/SharedArray.h"
#include "DRM_V3D.h"


namespace QPULib {
namespace v3d {

class Dispatcher {
  using Code  = SharedArrayBase; //  ISharedArray; //<uint64_t>;
  using Array = ISharedArray; //<uint32_t>;

public:
	Dispatcher(
		DRM_V3D &drm,
		BoHandles &bo_handles,
		int timeout_sec);

	~Dispatcher(); 

	void dispatch(
		Code &code,
		Array &uniforms,
		WorkGroup workgroup,
		uint32_t wgs_per_sg,
		uint32_t thread);

private:
	DRM_V3D   &m_drm;
	BoHandles  &m_bo_handles;
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
  using Code  = SharedArrayBase; //  ISharedArray; //<uint64_t>;
  using Array = ISharedArray; //<uint32_t>;

public:
	Dispatcher compute_shader_dispatcher(int timeout_sec= 10);

	void add_bo(SharedArray<uint32_t> &bo) {
		m_bo_handles.push_back(bo.getHandle());
	}

	void execute(
		Code &code,
		Array &uniforms,
		int thread = 1,
		int timeout_sec = 10,
		WorkGroup workgroup = (1, 1, 0),
		int wgs_per_sg = 3);  // Has an effect on number of registers used per QPU (max 16)

private:
	DRM_V3D m_drm;
	BoHandles m_bo_handles;

};  // class Driver

}  // v3d
}  // QPULib

#endif // _VC6_DRIVER_H_
