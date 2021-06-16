#ifndef _V3DLIB_V3D_INSTR_MNEMONICS_H
#define _V3DLIB_V3D_INSTR_MNEMONICS_H
#include "Instr.h"

namespace V3DLib {
namespace v3d {
namespace instr {

class Mnemonic : public Instr {
  using Parent = Instr;

public:
  Mnemonic() : Parent() {}
  Mnemonic(v3d_qpu_add_op op, Location const &dst, Source const &a, Source const &b);

  Mnemonic &pushz();
  Mnemonic &pushc();
  Mnemonic &pushn();

  Mnemonic &thrsw();
  Mnemonic &ldvary();
  Mnemonic &ldunif();
  Mnemonic &ldunifa();
  Mnemonic &ldvpm();

  Mnemonic &ldunifarf(Location const &loc);
  Mnemonic &ldunifrf(Location const &loc);
  Mnemonic &ldtmu(Location const &loc);


  //
  // Conditional execution of instructions
  // NOTE: These are meant for mnemonics when generating v3d instructions directly.
  //       It's really not a good idea to use them in the translation code (has bitten me).
  // 
  Mnemonic &ifa();
  Mnemonic &ifna();
  Mnemonic &ifb();
  Mnemonic &ifnb();

  Mnemonic &norn();
  Mnemonic &nornn();
  Mnemonic &norc();
  Mnemonic &nornc();
  Mnemonic &norz();

  Mnemonic &andn();
  Mnemonic &andz();
  Mnemonic &andc();
  Mnemonic &andnc();
  Mnemonic &andnn();

  // For branch instructions
  Mnemonic &a0();
  Mnemonic &na0();
  Mnemonic &alla();
  Mnemonic &allna();
  Mnemonic &anya();
  Mnemonic &anyaq();
  Mnemonic &anyap();
  Mnemonic &anyna();
  Mnemonic &anynaq();
  Mnemonic &anynap();


  // Regular mul mnemonics
#define REGULAR_INSTR(mnemonic, op) \
Mnemonic &mnemonic(Location const &dst, Source const &srca, Source const &srcb) \
  { mul_alu_set(op, dst, srca, srcb); return *this; }

  REGULAR_INSTR(add,    V3D_QPU_M_ADD)
  REGULAR_INSTR(sub,    V3D_QPU_M_SUB)
  REGULAR_INSTR(fmul,   V3D_QPU_M_FMUL)
  REGULAR_INSTR(smul24, V3D_QPU_M_SMUL24)
  REGULAR_INSTR(vfmul,  V3D_QPU_M_VFMUL)

#undef REGULAR_INSTR

  // Other mul mnemonics
  Mnemonic &nop();
  Mnemonic &mov(Location const &dst, Source const &src);
  Mnemonic &mov(uint8_t rf_addr, Register const &reg);
  Mnemonic &fmov(Location const &dst, Source const &src);

  Mnemonic &rotate(Location const &dst, Location const &a, SmallImm const &b);
  Mnemonic &rotate(Location const &dst, Location const &a, Location const &b);

private:
  bool m_doing_add = true;

  void mul_alu_set(v3d_qpu_mul_op op, Location const &dst, Source const &srca, Source const &srcb);
  void set_c(v3d_qpu_cond val);
  void set_uf(v3d_qpu_uf val);
  void set_pf(v3d_qpu_pf val);
  void set_sig_addr(Location const &loc);
};

}  // namespace instr

using Mnemonics = Instructions;

inline Instructions &operator<<(Instructions &lhs, instr::Mnemonic const &rhs) {
  lhs << (instr::Instr const &) rhs;
  return lhs;
}


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
// Regular Mnemonics
//
// These all follow exactly the same form:
//  - 1 destination register
//  - 2 source registers which can be any legal type
//
// Notes:
// - Bitwise operations have prefix 'b' because the expected names are c++ keywords.
// - faddnf() Logically the same as faddf() with mux a and b reversed.
//   fmin/fmax have the same relation.
///////////////////////////////////////////////////////////////////////////////

#define REGULAR_INSTR(mnemonic, op) \
inline Mnemonic mnemonic(Location const &dst, Source const &a, Source const &b) { return Mnemonic(op, dst, a, b); }


REGULAR_INSTR(shl,    V3D_QPU_A_SHL)
REGULAR_INSTR(shr,    V3D_QPU_A_SHR)
REGULAR_INSTR(asr,    V3D_QPU_A_ASR)
REGULAR_INSTR(add,    V3D_QPU_A_ADD)
REGULAR_INSTR(sub,    V3D_QPU_A_SUB)
REGULAR_INSTR(fsub,   V3D_QPU_A_FSUB)
REGULAR_INSTR(fadd,   V3D_QPU_A_FADD)
REGULAR_INSTR(faddnf, V3D_QPU_A_FADDNF)  // See note in header
REGULAR_INSTR(vfmin,  V3D_QPU_A_VFMIN)
REGULAR_INSTR(bor,    V3D_QPU_A_OR)
REGULAR_INSTR(bxor,   V3D_QPU_A_XOR)
REGULAR_INSTR(band,   V3D_QPU_A_AND)
REGULAR_INSTR(fmax,   V3D_QPU_A_FMAX)
REGULAR_INSTR(fcmp,   V3D_QPU_A_FCMP)
REGULAR_INSTR(vfpack, V3D_QPU_A_VFPACK)
REGULAR_INSTR(min,    V3D_QPU_A_MIN)
REGULAR_INSTR(max,    V3D_QPU_A_MAX)

#undef REGULAR_INSTR

///////////////////////////////////////////////////////////////////////////////
// Other Mnemonics
///////////////////////////////////////////////////////////////////////////////

Mnemonic nop();
Mnemonic tidx(Location const &reg);
Mnemonic eidx(Location const &reg);

Mnemonic itof(Location const &dst, Location const &a);
Mnemonic ftoi(Location const &dst, Location const &a);

Mnemonic mov(Location const &dst, Source const &a);

v3d_qpu_waddr const syncb = V3D_QPU_WADDR_SYNCB;  // Needed for barrierid()

Mnemonic barrierid(v3d_qpu_waddr waddr);

Mnemonic vpmsetup(Register const &reg2);

Mnemonic ffloor(Location const &dst, Source const &srca);
Mnemonic flpop(RFAddress rf_addr1, RFAddress rf_addr2);

Mnemonic fdx(Location const &dst, Location const &srca);
Mnemonic vflb(Location const &dst);
Mnemonic tmuwt();

Mnemonic ldvpmg_in(Location const &dst, Location const &a, Location const &b);
Mnemonic stvpmv(SmallImm const &a, Location const &b);
Mnemonic sampid(Location const &dst);

Mnemonic rotate(Location const &dst, Location const &a, Location const &b);
Mnemonic rotate(Location const &dst, Location const &a, SmallImm const &b);


///////////////////////////////////////////////////////////////////////////////
// Branch instructions
///////////////////////////////////////////////////////////////////////////////

Mnemonic branch(int target, int current);
Mnemonic branch(int target, bool relative);
Mnemonic bb(Location const &loc1);
Mnemonic bb(BranchDest const &loc1);
Mnemonic bb(uint32_t addr);
Mnemonic bu(uint32_t addr, Location const &loc2);
Mnemonic bu(BranchDest const &loc1, Location const &loc2);


///////////////////////////////////////////////////////////////////////////////
// SFU instructions - NOT WORKING, they return nothing
///////////////////////////////////////////////////////////////////////////////

Mnemonic brecip(Location const &dst, Location const &a);
Mnemonic brsqrt(Location const &dst, Location const &a);
Mnemonic brsqrt2(Location const &dst, Location const &a);
Mnemonic bsin(Location const &dst, Location const &a);
Mnemonic bexp(Location const &dst, Location const &a);
Mnemonic blog(Location const &dst, Location const &a);


///////////////////////////////////////////////////////////////////////////////
// Aggregated Instructions
///////////////////////////////////////////////////////////////////////////////

Mnemonics fsin(Location const &dst, Source const &a);

}  // instr
}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_MNEMONICS_H
