#include "SharedArray.h"
#include <sys/mman.h>
#include "../../debug.h"

namespace QPULib {
namespace v3d {

SharedArrayBase::~SharedArrayBase() {
	breakpoint
	int res = munmap(usraddr, m_size);
	assert(res == 0);
} 

}  // namespace v3d
}  // namespace QPULib

