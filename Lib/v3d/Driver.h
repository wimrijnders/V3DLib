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

	bool dispatch(
		uint32_t code_phyaddr,
		uint32_t code_handle,  // Only passed in for check
		uint32_t unif_phyaddr,
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

	void add_bo(uint32_t bo_handle) {
		m_bo_handles.push_back(bo_handle);
	}

	void execute(
		Code &code,
		Array *uniforms = nullptr,
		int thread = 1,
		int timeout_sec = 10,
		WorkGroup workgroup = (1, 1, 0),
		int wgs_per_sg = 3);


	bool execute_intern(
		uint32_t code_phyaddr,
		uint32_t code_handle,  // Only passed in for check
		uint32_t unif_phyaddr,
		int thread = 1,
		int timeout_sec = 10,
		WorkGroup workgroup = (1, 1, 0),
		int wgs_per_sg = 3);  // Has an effect on number of registers used per QPU (max 16)

private:
	DRM_V3D   m_drm;
	BoHandles m_bo_handles;

};  // class Driver

// Legacy call(s)
bool v3d_submit_csd(uint32_t phyaddr, uint32_t handle, uint32_t uniforms = 0);
inline bool v3d_submit_csd(SharedArrayBase &codeMem) {
	return v3d_submit_csd(codeMem.getPhyAddr(), codeMem.getHandle());
}

}  // v3d
}  // QPULib

#endif // _VC6_DRIVER_H_
