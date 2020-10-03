#ifndef _QPULIB_TARGET_PRETTY_H_
#define _QPULIB_TARGET_PRETTY_H_

#include <stdio.h>
#include "Target/Syntax.h"

namespace QPULib {

const char *pretty_instr_tag(InstrTag tag);

// Pretty printer for the QPULib target language
void pretty(FILE *f, Instr instr, int index);

}  // namespace QPULib

#endif  // _QPULIB_TARGET_PRETTY_H_
