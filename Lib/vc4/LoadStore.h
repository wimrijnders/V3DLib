#ifndef _V3DLIB_LOADSTORE_H_
#define _V3DLIB_LOADSTORE_H_

#include "Common/Seq.h"
#include "Target/Syntax.h"
#include "Source/Syntax.h"

namespace V3DLib {

Seq<Instr> genSetupVPMLoad(int n, int addr, int hor, int stride);
Seq<Instr> genSetupVPMLoad(int n, Reg addr, int hor, int stride);

Instr genSetupVPMStore(int addr, int hor, int stride);
Seq<Instr>  genSetupVPMStore(Reg addr, int hor, int stride);

Instr genSetupDMALoad(int numRows, int rowLen, int hor, int vpitch, int vpmAddr);
Seq<Instr>  genSetupDMALoad(int numRows, int rowLen, int hor, int vpitch, Reg vpmAddr);
Instr genStartDMALoad(Reg memAddr);
Instr genWaitDMALoad(bool might_be_end = true);

Instr genSetupDMAStore(int numRows, int rowLen, int hor, int vpmAddr);
Seq<Instr> genSetupDMAStore(int numRows, int rowLen, int hor, Reg vpmAddr);
Instr genStartDMAStore(Reg memAddr);
Instr genWaitDMAStore();

Instr genSetReadPitch(int pitch);
Seq<Instr> genSetReadPitch(Reg pitch);

Instr genSetWriteStride(int stride);
Seq<Instr> genSetWriteStride(Reg stride);

}  // namespace V3DLib

#endif  // _V3DLIB_LOADSTORE_H_
