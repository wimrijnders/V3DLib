#ifndef _VC6_DRIVER_H_
#define _VC6_DRIVER_H_

#ifdef QPU_MODE

#include "Common/SharedArray.h"
#include "v3d.h"


namespace V3DLib {
namespace v3d {

/**
 *
 */
class Driver {
  using BoHandles  = std::vector<uint32_t>;
  using Code       = SharedArray<uint64_t>;
  using UniformArr = SharedArray<uint32_t>;

public:
  void add_bo(BufferObject const &bo) {
    m_bo_handles.push_back(bo.getHandle());
  }

  bool execute(Code &code, UniformArr *uniforms = nullptr, uint32_t thread = 1);

private:
  BoHandles m_bo_handles;
};  // class Driver

}  // v3d
}  // V3DLib

#endif  // QPU_MODE

#endif  // _VC6_DRIVER_H_
