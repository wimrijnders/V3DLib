#ifndef _V3DLIB_ENCODE_H_
#define _V3DLIB_ENCODE_H_
#include <stdint.h>
#include "Target/Syntax.h"
#include "Common/Seq.h"

namespace V3DLib {
namespace vc4 {

uint64_t encode(Instr instr);
void encode(Seq<Instr>* instrs, Seq<uint32_t>* code);

}  // namespace vc4
}  // namespace V3DLib

#endif  // _V3DLIB_ENCODE_H_
