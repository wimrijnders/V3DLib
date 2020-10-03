#ifndef _QPULIB_LOADSTORE_H_
#define _QPULIB_LOADSTORE_H_

#include "Common/Seq.h"
#include "Target/Syntax.h"
#include "Source/Syntax.h"

namespace QPULib {

void genSetupVPMLoad(Seq<Instr>* instrs, int n, int addr, int hor, int stride);
void genSetupVPMLoad(Seq<Instr>* instrs, int n, Reg addr, int hor, int stride);

Instr  genSetupVPMStore(int addr, int hor, int stride);
void genSetupVPMStore(Seq<Instr>* instrs, Reg addr, int hor, int stride);

Instr genSetupDMALoad(int numRows, int rowLen, int hor, int vpitch, int vpmAddr);
void genSetupDMALoad(Seq<Instr>* instrs, int numRows, int rowLen, int hor, int vpitch, Reg vpmAddr);
void genStartDMALoad(Seq<Instr>* instrs, Reg memAddr);
Instr genWaitDMALoad(bool might_be_end = true);

void genSetupDMAStore(Seq<Instr>* instrs, int numRows, int rowLen, int hor, int vpmAddr);
void genSetupDMAStore(Seq<Instr>* instrs, int numRows, int rowLen, int hor, Reg vpmAddr);
Instr genStartDMAStore(Reg memAddr);
void genWaitDMAStore(Seq<Instr>* instrs);

Instr genSetReadPitch(int pitch);
void genSetReadPitch(Seq<Instr>* instrs, Reg pitch);

void genSetWriteStride(Seq<Instr>* instrs, int stride);
void genSetWriteStride(Seq<Instr>* instrs, Reg stride);

}  // namespace QPULib

#endif  // _QPULIB_LOADSTORE_H_
