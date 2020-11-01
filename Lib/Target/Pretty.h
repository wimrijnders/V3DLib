#ifndef _QPULIB_TARGET_PRETTY_H_
#define _QPULIB_TARGET_PRETTY_H_
#include <string>
#include "Target/Syntax.h"

namespace QPULib {

const char *pretty_instr_tag(InstrTag tag);
std::string pretty_instr(Instr const &instr);

}  // namespace QPULib

#endif  // _QPULIB_TARGET_PRETTY_H_
