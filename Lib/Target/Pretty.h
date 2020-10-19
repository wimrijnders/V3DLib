#ifndef _QPULIB_TARGET_PRETTY_H_
#define _QPULIB_TARGET_PRETTY_H_
#include <stdio.h>
#include "Target/Syntax.h"

namespace QPULib {

const char *pretty_instr_tag(InstrTag tag);

std::string pretty(Instr const &instr, int index = -1, bool with_comments = false);

}  // namespace QPULib

#endif  // _QPULIB_TARGET_PRETTY_H_
