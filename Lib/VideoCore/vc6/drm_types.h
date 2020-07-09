//
// Placeholder definitions for certain data types.
//
// These should be filled out in due time
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _VC6_DRM_TYPES_H_
#define _VC6_DRM_TYPES_H_
#include <stdint.h>

namespace QPULib {
namespace vc6 {

class Uniforms {};
typedef uint32_t Cfg[6];

typedef uint32_t Coef[4];
typedef uint32_t *BoHandles;


class Code {
public:
	uint32_t *addresses() {
		assert(false);  // TODO
		return nullptr;
	}
};


struct WorkGroup {
	uint32_t wg_x = 0;
	uint32_t wg_y = 0;
	uint32_t wg_z = 0;

	WorkGroup(uint32_t x = 16, uint32_t y = 1, uint32_t z = 1) :
		wg_x(x),
		wg_y(y),
		wg_z(z) {}

  uint32_t wg_size() { return wg_x * wg_y * wg_z; }
};


}  // vc6
}  // QPULib


#endif  // _VC6_DRM_TYPES_H_
