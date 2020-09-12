#ifndef _VC6_DRIVER_H_
#define _VC6_DRIVER_H_
#include "Common/SharedArray.h"
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
	using BoHandles = std::vector<uint32_t>;

public:
	void add_bo(BufferObject const &bo) {
		m_bo_handles.push_back(bo.getHandle());
	}

	void add_bo(uint32_t bo_handle) {
		m_bo_handles.push_back(bo_handle);
	}

	bool execute(SharedArray<uint64_t> &code, SharedArray<uint32_t> *uniforms = nullptr, uint32_t thread = 1);

private:
	BoHandles m_bo_handles;
  int       m_timeout_sec = 10;
};  // class Driver

}  // v3d
}  // QPULib

#endif // _VC6_DRIVER_H_
