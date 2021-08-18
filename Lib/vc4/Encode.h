#ifndef _V3DLIB_ENCODE_H_
#define _V3DLIB_ENCODE_H_
#include <stdint.h>
#include "Target/instr/Instr.h"

namespace V3DLib {
namespace vc4 {

using CodeList = Seq<uint64_t>;

CodeList encode(Instr::List &instrs);

}  // namespace vc4
}  // namespace V3DLib

#endif  // _V3DLIB_ENCODE_H_
