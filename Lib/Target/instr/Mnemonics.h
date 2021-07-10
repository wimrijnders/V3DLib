#ifndef _V3DLIB_TARGET_INSTR_MNEMONICS_H_
#define _V3DLIB_TARGET_INSTR_MNEMONICS_H_
#include "Instr.h"

namespace V3DLib {
namespace Target {
namespace instr {

extern Reg const None;
extern Reg const ACC0;
extern Reg const ACC1;
extern Reg const ACC2;
extern Reg const ACC3;
extern Reg const ACC4;
extern Reg const ACC5;
extern Reg const UNIFORM_READ;
extern Reg const QPU_ID;
extern Reg const ELEM_ID;
extern Reg const TMU0_S;
extern Reg const VPM_WRITE;
extern Reg const VPM_READ;
extern Reg const WR_SETUP;
extern Reg const RD_SETUP;
extern Reg const DMA_LD_WAIT;
extern Reg const DMA_ST_WAIT;
extern Reg const DMA_LD_ADDR;
extern Reg const DMA_ST_ADDR;
extern Reg const SFU_RECIP;
extern Reg const SFU_RECIPSQRT;
extern Reg const SFU_EXP;
extern Reg const SFU_LOG;

// Following registers are synonyms for v3d code generation,
// to better indicate the intent. Definitions of vc4 concepts
// are reused here, in order to prevent the code getting into a mess.
extern Reg const TMUD;
extern Reg const TMUA;

Reg rf(uint8_t index);

Instr bor(Reg dst, RegOrImm const &srcA, RegOrImm const &srcB);
Instr band(Reg dst, Reg srcA, Reg srcB);
Instr band(Reg dst, Reg srcA, int n);
Instr bxor(Var dst, RegOrImm srcA, int n);
Instr mov(Reg dst, RegOrImm const &src);
Instr shl(Reg dst, Reg srcA, int val);
Instr add(Reg dst, Reg srcA, Reg srcB);
Instr add(Reg dst, Reg srcA, int n);
Instr sub(Reg dst, Reg srcA, int n);
Instr shr(Reg dst, Reg srcA, int n);
Instr li(Reg dst, Imm const &src);
Instr branch(Label label);
Instr label(Label in_label);

//
// SFU functions
//
Instr::List recip(Var dst, Var srcA);
Instr::List recipsqrt(Var dst, Var srcA);
Instr::List bexp(Var dst, Var srcA);
Instr::List blog(Var dst, Var srcA);

Instr recv(Reg dst);

// v3d only
Instr tmuwt();

}  // namespace instr
}  // namespace Target
}  // namespace V3DLib

#endif  // _V3DLIB_TARGET_INSTR_MNEMONICS_H_
