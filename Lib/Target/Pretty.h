#ifndef _V3DLIB_TARGET_PRETTY_H_
#define _V3DLIB_TARGET_PRETTY_H_
#include <string>
#include "Target/Syntax.h"

namespace V3DLib {

const char *pretty_instr_tag(InstrTag tag);
std::string pretty_instr(Instr const &instr);

}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_PRETTY_H_
