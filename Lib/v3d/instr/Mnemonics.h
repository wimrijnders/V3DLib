#ifndef _V3DLIB_V3D_INSTR_MNEMONICS_H
#define _V3DLIB_V3D_INSTR_MNEMONICS_H
#include "Instr.h"

namespace V3DLib {
namespace v3d {
namespace instr {


class Source {
public:
  Source(V3DLib::RegOrImm const &rhs);

  bool is_location() const { return m_is_location; }
  Location const &location() const;
  SmallImm const &small_imm() const;

private:
  bool m_is_location = false;
  std::unique_ptr<Location> m_location;
  SmallImm m_small_imm;
};

///////////////////////////////////////////////////////////////////////////////
// Class Mnemonic 
///////////////////////////////////////////////////////////////////////////////

class Mnemonic : public Instr {
  using Parent = Instr;

public:
  Mnemonic() : Parent() {}
  Mnemonic(v3d_qpu_add_op op, Location const &dst, Source const &a, Source const &b);
  Mnemonic(v3d_qpu_add_op op, Location const &dst, Location const &srca, Location const &srcb);
  Mnemonic(v3d_qpu_add_op op, Location const &dst, Location const &srca, SmallImm const &immb);
  Mnemonic(v3d_qpu_add_op op, Location const &dst, SmallImm const &imma, Location const &srcb);
  Mnemonic(v3d_qpu_add_op op, Location const &dst, SmallImm const &imma, SmallImm const &immb);

  void alu_add_set(Location const &dst, Location const &a, Location const &b); 
  void alu_add_set(Location const &dst, SmallImm const &a, Location const &b);
  void alu_add_set(Location const &dst, Location const &a, SmallImm const &b);
  void alu_add_set(Location const &dst, SmallImm const &a, SmallImm const &b);

  Mnemonic &pushz();
  Mnemonic &pushc();
  Mnemonic &pushn();

  Mnemonic &thrsw();
  Mnemonic &ldvary();
  Mnemonic &ldunif();
  Mnemonic &ldunifa();
  Mnemonic &ldunifarf(Location const &loc);
  Mnemonic &ldunifrf(RFAddress const &loc);

  Mnemonic &ldtmu(Register const &reg);
  Mnemonic &ldvpm();

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

  //
  // Calls to set the mul part of the instruction
  //
  Mnemonic &nop();

  Mnemonic &mov(Location const &dst, SmallImm const &imm);
  Mnemonic &mov(uint8_t rf_addr, Register const &reg);
  Mnemonic &mov(Location const &loc1, Location const &loc2);
  Mnemonic &fmov(Location const &dst, SmallImm const &imma);

  Mnemonic &add(Location const &dst, Location const &srca, Location const &srcb);
  Mnemonic &sub(Location const &dst, Location const &srca, Location const &srcb);
  Mnemonic &sub(Location const &dst, Location const &srca, SmallImm const &immb);

  Mnemonic &fmul(Location const &loc1, Location const &loc2, Location const &loc3);
  Mnemonic &fmul(Location const &loc1, SmallImm imm2, Location const &loc3);
  Mnemonic &fmul(Location const &loc1, Location const &loc2, SmallImm const &imm3);
  Mnemonic &smul24(Location const &dst, Location const &loca, Location const &locb); 
  Mnemonic &smul24(Location const &dst, SmallImm const &imma, Location const &locb); 
  Mnemonic &smul24(Location const &dst, Location const &loca, SmallImm const &immb); 
  Mnemonic &vfmul(Location const &rf_addr1, Register const &reg2, Register const &reg3);

  Mnemonic &rotate(Location const &dst, Location const &a, SmallImm const &b);
  Mnemonic &rotate(Location const &dst, Location const &a, Location const &b);

  // TODO check if needed
  void doing_mul() {
    assert(m_doing_add);
    m_doing_add = false;
  }

private:
  bool m_doing_add = true;

  void set_c(v3d_qpu_cond val);
  void set_uf(v3d_qpu_uf val);
  void set_pf(v3d_qpu_pf val);
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
// Instructions
///////////////////////////////////////////////////////////////////////////////

Mnemonic nop();
Mnemonic tidx(Location const &reg);
Mnemonic eidx(Location const &reg);

Mnemonic itof(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic ftoi(Location const &dst, Location const &a, SmallImm const &b);

Mnemonic mov(Location const &dst, Source const &a);
Mnemonic mov(Location const &dst, SmallImm const &a);
Mnemonic mov(Location const &dst, Location const &a);

Mnemonic shr(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic shl(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic shl(Location const &dst, SmallImm const &a, SmallImm const &b);
Mnemonic shl(Location const &dst, Location const &a, Location const &b);
Mnemonic shl(Location const &dst, SmallImm const &a, Location const &b);
Mnemonic shl(Location const &dst, Location const &a, Location const &b);
Mnemonic asr(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic asr(Location const &dst, Location const &a, Location const &b);
Mnemonic add(Location const &dst, Location const &a, Location const &b);
Mnemonic add(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic add(Location const &dst, SmallImm const &a, Location const &b);
Mnemonic sub(Location const &dst, Location const &a, Location const &b);
Mnemonic sub(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic sub(Location const &dst, SmallImm const &a, Location const &b);
Mnemonic fsub(Location const &dst, Location const &a, Location const &b);
Mnemonic fsub(Location const &dst, SmallImm const &a, Location const &b);
Mnemonic fsub(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic fadd(Location const &dst, Location const &a, Location const &b);
Mnemonic fadd(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic fadd(Location const &dst, SmallImm const &a, Location const &b);
Mnemonic faddnf(Location const &dst, Location const &a, Location const &b);
Mnemonic faddnf(Location const &dst, SmallImm const &a, Location const &b);

Mnemonic bor( Location const &dst, Location const &a, Location const &b);
Mnemonic bor( Location const &dst, Location const &a, SmallImm const &b);
Mnemonic bor( Location const &dst, SmallImm const &a, SmallImm const &b);
Mnemonic band(Location const &dst, Location const &a, Location const &b);
Mnemonic band(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic bxor(Location const &dst, Location const &a, SmallImm const &b);
Mnemonic bxor(Location const &dst, SmallImm const &a, SmallImm const &b);

Mnemonic fmax(Location const &dst, Location const &a, Location const &b);
Mnemonic fcmp(Location const &loc1, Location const &a, Location const &b);
Mnemonic vfpack(Location const &dst, Location const &a, Location const &b);
Mnemonic vfmin(Location const &dst, SmallImm const &a, Location const &b);
Mnemonic vfmin(Location const &dst, Location const &a, Location const &b);
Mnemonic min(Location const &dst, Location const &a, Location const &b);
Mnemonic max(Location const &dst, Location const &a, Location const &b);

v3d_qpu_waddr const syncb = V3D_QPU_WADDR_SYNCB;  // Needed for barrierid()

Mnemonic barrierid(v3d_qpu_waddr waddr);

Mnemonic vpmsetup(Register const &reg2);

Mnemonic ffloor(Location const &dst, Location const &srca);
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
