#ifndef _QPULIB_LOADSTORE_H_
#define _QPULIB_LOADSTORE_H_

#include "Common/Seq.h"
#include "Target/Syntax.h"
#include "Source/Syntax.h"

namespace QPULib {

Seq<Instr> genSetupVPMLoad(int n, int addr, int hor, int stride);
Seq<Instr> genSetupVPMLoad(int n, Reg addr, int hor, int stride);

Instr genSetupVPMStore(int addr, int hor, int stride);
Seq<Instr>  genSetupVPMStore(Reg addr, int hor, int stride);

Instr genSetupDMALoad(int numRows, int rowLen, int hor, int vpitch, int vpmAddr);
Seq<Instr>  genSetupDMALoad(int numRows, int rowLen, int hor, int vpitch, Reg vpmAddr);
Instr genStartDMALoad(Reg memAddr);
Instr genWaitDMALoad(bool might_be_end = true);

void genSetupDMAStore(Seq<Instr>* instrs, int numRows, int rowLen, int hor, int vpmAddr);
void genSetupDMAStore(Seq<Instr>* instrs, int numRows, int rowLen, int hor, Reg vpmAddr);
Instr genStartDMAStore(Reg memAddr);
Instr genWaitDMAStore();

Instr genSetReadPitch(int pitch);
void genSetReadPitch(Seq<Instr>* instrs, Reg pitch);

void genSetWriteStride(Seq<Instr>* instrs, int stride);
void genSetWriteStride(Seq<Instr>* instrs, Reg stride);

}  // namespace QPULib

#endif  // _QPULIB_LOADSTORE_H_
