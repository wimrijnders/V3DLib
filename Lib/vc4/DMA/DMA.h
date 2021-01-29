#ifndef _V3DLIB_VC4_DMA_DMA_H_
#define _V3DLIB_VC4_DMA_DMA_H_
#include <string>
#include "Source/Stmt.h"

namespace V3DLib {
namespace DMA {

std::string pretty(int indent, Stmt::Ptr s);
std::string disp(StmtTag tag);

}  // namespace DMA
}  // namespace V3DLib

#endif  // _V3DLIB_VC4_DMA_DMA_H_
