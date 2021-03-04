#ifndef _V3DLIB_V3D_INSTR_INSTR_H
#define _V3DLIB_V3D_INSTR_INSTR_H
#include <cstdint>
#include <string>
#include <vector>
#include "Support/InstructionComment.h"
#include "SmallImm.h"
#include "Register.h"
#include "RFAddress.h"

namespace V3DLib {
namespace v3d {
namespace instr {

using rf = RFAddress;
using si = SmallImm;


class Instr : public v3d_qpu_instr, public InstructionComment {

  // Label support
private:
  bool m_is_label = false;
  int  m_label    = -1;     // Also use for branch with label

public:
  bool is_label()        const { return m_is_label; }
  int  label()           const { assert(m_is_label); return m_label; }
  bool is_branch_label() const;
  int  branch_label()    const;

  void is_label(bool val) { m_is_label = val; }
  void label(int val);
  void label_to_target(int offset);
  // End Label support


public:
  Instr(uint64_t in_code = NOP);
  Instr(v3d_qpu_add_op op, Location const &dst, Location const &srca, Location const &srcb);
  Instr(v3d_qpu_add_op op, Location const &dst, Location const &srca, SmallImm const &immb);
  Instr(v3d_qpu_add_op op, Location const &dst, SmallImm const &imma, Location const &srcb);
  Instr(v3d_qpu_add_op op, Location const &dst, SmallImm const &imma, SmallImm const &immb);

  Instr &header(std::string const &msg) { InstructionComment::header(msg);  return *this; }
  Instr &comment(std::string msg)       { InstructionComment::comment(msg); return *this; }

  std::string dump() const; 
  std::string mnemonic(bool with_comments = false) const;
  uint64_t code() const;
  static std::string dump(uint64_t in_code);
  static std::string mnemonic(uint64_t in_code);
  static std::string mnemonics(std::vector<uint64_t> in_code);

  operator uint64_t() const { return code(); }

  bool add_nop()    const { return alu.add.op == V3D_QPU_A_NOP; }
  bool mul_nop()    const { return alu.mul.op == V3D_QPU_M_NOP; }
  bool add_nocond() const { return flags.ac == V3D_QPU_COND_NONE; }
  bool mul_nocond() const { return flags.mc == V3D_QPU_COND_NONE; }

  Instr &pushz();
  Instr &pushc();
  Instr &pushn();

  Instr &thrsw();
  Instr &ldvary();
  Instr &ldunif();
  Instr &ldunifa();
  Instr &ldunifarf(Location const &loc);
  Instr &ldunifrf(RFAddress const &loc);
  Instr &ldtmu(Register const &reg);
  Instr &ldvpm();

  // Conditional execution of instructions
  Instr &ifa();
  Instr &ifna();
  Instr &ifb();
  Instr &ifnb();

  Instr &norn();
  Instr &nornn();
  Instr &norc();
  Instr &nornc();
  Instr &norz();

  Instr &andn();
  Instr &andz();
  Instr &andc();
  Instr &andnc();
  Instr &andnn();

  // For branch instructions
  Instr &a0();
  Instr &na0();
  Instr &alla();
  Instr &allna();
  Instr &anya();
  Instr &anyaq();
  Instr &anyap();
  Instr &anyna();
  Instr &anynaq();
  Instr &anynap();

  //
  // Calls to set the mul part of the instruction
  //
  Instr &nop();

  Instr &add(Location const &dst, Location const &srca, Location const &srcb);
  Instr &sub(Location const &dst, Location const &srca, Location const &srcb);

  Instr &mov(Location const &dst, SmallImm const &imm);
  Instr &mov(uint8_t rf_addr, Register const &reg);
  Instr &mov(Location const &loc1, Location const &loc2);
  Instr &fmov(Location const &dst, SmallImm const &imma);

  Instr &fmul(Location const &loc1, Location const &loc2, Location const &loc3);
  Instr &fmul(Location const &loc1, SmallImm imm2, Location const &loc3);
  Instr &fmul(Location const &loc1, Location const &loc2, SmallImm const &imm3);
  Instr &smul24(Location const &dst, Location const &loca, Location const &locb); 
  Instr &smul24(Location const &dst, SmallImm const &imma, Location const &locb); 
  Instr &smul24(Location const &dst, Location const &loca, SmallImm const &immb); 
  Instr &vfmul(Location const &rf_addr1, Register const &reg2, Register const &reg3);

  Instr &rotate(Location const &dst, Location const &a, SmallImm const &b);
  Instr &rotate(Location const &dst, Location const &a, Location const &b);

  static bool compare_codes(uint64_t code1, uint64_t code2);

  void alu_add_set_dst(Location const &dst); 
  void alu_mul_set_dst(Location const &dst); 
  void alu_add_set_reg_a(Location const &loc);
  void alu_add_set_reg_b(Location const &loc);
  void alu_mul_set_reg_a(Location const &loc);
  void alu_mul_set_reg_b(Location const &loc);
  void alu_set_imm(SmallImm const &imm);
  void alu_add_set_imm_a(SmallImm const &imm);
  void alu_add_set_imm_b(SmallImm const &imm);
  void alu_mul_set_imm_a(SmallImm const &imm);
  void alu_mul_set_imm_b(SmallImm const &imm);

  void alu_add_set(Location const &dst, Location const &a, Location const &b); 
  void alu_add_set(Location const &dst, SmallImm const &a, Location const &b);
  void alu_add_set(Location const &dst, Location const &a, SmallImm const &b);
  void alu_add_set(Location const &dst, SmallImm const &a, SmallImm const &b);
  void alu_mul_set(Location const &dst, Location const &a, Location const &b); 
  void alu_mul_set(Location const &dst, Location const &a, SmallImm const &b); 
  void alu_mul_set(Location const &dst, SmallImm const &a, Location const &b); 

  // ==================================================
  // Private State 
  // ==================================================

private:
  static uint64_t const NOP;
  bool m_doing_add = true;

  void init_ver() const;
  void init(uint64_t in_code);
  Instr &set_branch_condition(v3d_qpu_branch_cond cond);
  void set_c(v3d_qpu_cond val);
  void set_uf(v3d_qpu_uf val);
  void set_pf(v3d_qpu_pf val);

  bool raddr_a_is_safe(Location const &loc) const;

  std::string pretty_instr() const;
};


const uint8_t  vpm       = 14;
const uint32_t zero_addr = 0;

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
Instr asr(Location const &dst, Location const &a, SmallImm const &b);
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

}  // instr

//
// Some type definitions, for better understanding
//
using ByteCode = std::vector<uint64_t>; 

class Instructions : public std::vector<instr::Instr> {
  using Parent = std::vector<instr::Instr>;

public:
  Instructions() = default;
  Instructions(Parent const &rhs) : Parent(rhs) {}

  Instructions &header(std::string const &msg) { front().header(msg);  return *this; }

  Instructions &comment(std::string msg, bool to_front = true) {
    assert(!empty());

    if (to_front) {
      front().comment(msg);
    } else {
      back().comment(msg);
    }

    return *this;
  }
};

}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_INSTR_H
