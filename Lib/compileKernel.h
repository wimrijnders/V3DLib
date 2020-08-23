#ifndef _QPULIB_COMPILEKERNEL_H_
#define _QPULIB_COMPILEKERNEL_H_
#include "Common/Seq.h"
#include "Target/Syntax.h"  // Instr
#include "Source/Stmt.h"

namespace QPULib {

// Compile a kernel
void compileKernel(Seq<Instr>* targetCode, Stmt* s);

}  // namespace QPULib

#endif  // _QPULIB_COMPILEKERNEL_H_
