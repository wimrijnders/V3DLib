#ifndef _VC6_DRIVER_H_
#define _VC6_DRIVER_H_
#include "v3d/SharedArray.h"
#include "v3d.h"


namespace QPULib {
namespace v3d {

/**
 * NOTE: In python call, following was done in ctor:
 *
 *         self.bo_handles = np.array([self.memory.handle], dtype=np.uint32)
 *
 * TODO: check if this is relevant
 */
class Driver {
  using Array = ISharedArray;
	using BoHandles = std::vector<uint32_t>;

public:
	void add_bo(SharedArray<uint32_t> &bo) {
		m_bo_handles.push_back(bo.getHandle());
	}

	void add_bo(uint32_t bo_handle) {
		m_bo_handles.push_back(bo_handle);
	}

	void execute(
		BufferObject const &code,
		Array *uniforms = nullptr,
		int thread = 1);


	bool dispatch(
		uint32_t code_phyaddr,
		uint32_t code_handle,  // Only passed in for check
		uint32_t unif_phyaddr,
		uint32_t thread = 1);

private:
	BoHandles m_bo_handles;
  int       m_timeout_sec = 10;

};  // class Driver

// Legacy call(s)
bool v3d_submit_csd(uint32_t phyaddr, uint32_t handle, uint32_t uniforms = 0);
inline bool v3d_submit_csd(BufferObject const &codeMem) {
	return v3d_submit_csd(codeMem.getPhyAddr(), codeMem.getHandle());
}

}  // v3d
}  // QPULib

#endif // _VC6_DRIVER_H_
