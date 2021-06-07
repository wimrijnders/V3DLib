#ifndef _V3DLIB_V3D_INSTR_MNEMONICS_H
#define _V3DLIB_V3D_INSTR_MNEMONICS_H
#include "Instr.h"

namespace V3DLib {
namespace v3d {
namespace instr {

///////////////////////////////////////////////////////////////////////////////
// Registers 
///////////////////////////////////////////////////////////////////////////////

extern Register const r0;
extern Register const r1;
extern Register const r2;
extern Register const r3;
extern Register const r4;
extern Register const r5;
extern Register const tmua;
extern Register const tmud;
extern Register const tlb;
extern Register const recip;
extern Register const rsqrt;
extern Register const exp;
extern Register const log;
extern Register const sin;
extern Register const rsqrt2;

// For branch
extern BranchDest const lri;
extern Register const r_unif;
extern Register const a_unif;


const uint8_t  vpm       = 14;
const uint32_t zero_addr = 0;


///////////////////////////////////////////////////////////////////////////////
// Instructions
///////////////////////////////////////////////////////////////////////////////

Instr nop();
Instr tidx(Location const &reg);
Instr eidx(Location const &reg);

Instr itof(Location const &dst, Location const &a, SmallImm const &b);
Instr ftoi(Location const &dst, Location const &a, SmallImm const &b);

Instr mov(Location const &dst, SmallImm const &a);
Instr mov(Location const &dst, Location const &a);

Instr shr(Location const &dst, Location const &a, SmallImm const &b);
Instr shl(Location const &dst, Location const &a, SmallImm const &b);
Instr shl(Location const &dst, SmallImm const &a, SmallImm const &b);
Instr shl(Location const &dst, Location const &a, Location const &b);
Instr shl(Location const &dst, SmallImm const &a, Location const &b);
Instr shl(Location const &dst, Location const &a, Location const &b);
Instr asr(Location const &dst, Location const &a, SmallImm const &b);
Instr asr(Location const &dst, Location const &a, Location const &b);
Instr add(Location const &dst, Location const &a, Location const &b);
Instr add(Location const &dst, Location const &a, SmallImm const &b);
Instr add(Location const &dst, SmallImm const &a, Location const &b);
Instr sub(Location const &dst, Location const &a, Location const &b);
Instr sub(Location const &dst, Location const &a, SmallImm const &b);
Instr sub(Location const &dst, SmallImm const &a, Location const &b);
Instr fsub(Location const &dst, Location const &a, Location const &b);
Instr fsub(Location const &dst, SmallImm const &a, Location const &b);
Instr fsub(Location const &dst, Location const &a, SmallImm const &b);
Instr fadd(Location const &dst, Location const &a, Location const &b);
Instr fadd(Location const &dst, Location const &a, SmallImm const &b);
Instr fadd(Location const &dst, SmallImm const &a, Location const &b);
Instr faddnf(Location const &dst, Location const &a, Location const &b);
Instr faddnf(Location const &dst, SmallImm const &a, Location const &b);

Instr bor( Location const &dst, Location const &a, Location const &b);
Instr bor( Location const &dst, Location const &a, SmallImm const &b);
Instr bor( Location const &dst, SmallImm const &a, SmallImm const &b);
Instr band(Location const &dst, Location const &a, Location const &b);
Instr band(Location const &dst, Location const &a, SmallImm const &b);
Instr bxor(Location const &dst, Location const &a, SmallImm const &b);
Instr bxor(Location const &dst, SmallImm const &a, SmallImm const &b);

Instr fmax(Location const &dst, Location const &a, Location const &b);
Instr fcmp(Location const &loc1, Location const &a, Location const &b);
Instr vfpack(Location const &dst, Location const &a, Location const &b);
Instr vfmin(Location const &dst, SmallImm const &a, Location const &b);
Instr vfmin(Location const &dst, Location const &a, Location const &b);
Instr min(Location const &dst, Location const &a, Location const &b);
Instr max(Location const &dst, Location const &a, Location const &b);

v3d_qpu_waddr const syncb = V3D_QPU_WADDR_SYNCB;  // Needed for barrierid()

Instr barrierid(v3d_qpu_waddr waddr);

Instr vpmsetup(Register const &reg2);

Instr ffloor(Location const &dst, Location const &srca);
Instr flpop(RFAddress rf_addr1, RFAddress rf_addr2);

Instr fdx(Location const &dst, Location const &srca);
Instr vflb(Location const &dst);
Instr tmuwt();

Instr ldvpmg_in(Location const &dst, Location const &a, Location const &b);
Instr stvpmv(SmallImm const &a, Location const &b);
Instr sampid(Location const &dst);

Instr rotate(Location const &dst, Location const &a, Location const &b);
Instr rotate(Location const &dst, Location const &a, SmallImm const &b);


///////////////////////////////////////////////////////////////////////////////
// Branch Instructions
///////////////////////////////////////////////////////////////////////////////

Instr branch(int target, int current);
Instr branch(int target, bool relative);
Instr bb(Location const &loc1);
Instr bb(BranchDest const &loc1);
Instr bb(uint32_t addr);
Instr bu(uint32_t addr, Location const &loc2);
Instr bu(BranchDest const &loc1, Location const &loc2);


///////////////////////////////////////////////////////////////////////////////
// SFU Instructions - NOT WORKING, they return nothing
///////////////////////////////////////////////////////////////////////////////

Instr brecip(Location const &dst, Location const &a);
Instr brsqrt(Location const &dst, Location const &a);
Instr brsqrt2(Location const &dst, Location const &a);
Instr bsin(Location const &dst, Location const &a);
Instr bexp(Location const &dst, Location const &a);
Instr blog(Location const &dst, Location const &a);


///////////////////////////////////////////////////////////////////////////////
// Aggregated Instructions
///////////////////////////////////////////////////////////////////////////////

Instructions fsin(Location const &dst, Location const &a);


}  // instr
}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_MNEMONICS_H
