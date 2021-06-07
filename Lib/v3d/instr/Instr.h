#ifndef _V3DLIB_V3D_INSTR_INSTR_H
#define _V3DLIB_V3D_INSTR_INSTR_H
#include <cstdint>
#include <string>
#include <vector>
#include "Support/InstructionComment.h"
#include "SmallImm.h"
#include "Register.h"
#include "Encode.h"
#include "Target/instr/ALUInstruction.h"

namespace V3DLib {

class ALUInstruction;

namespace v3d {
namespace instr {

using rf = RFAddress;
using si = SmallImm;


/**
 * NOTE: branch condition is distinct from add/mul alu assign condition tags.
 *       Would it then be possible to combine them?? E.g. something like:
 * 
 *           bb(L0).ifna().a0();
 *           bb(L0).nop().ifna().a0();  // Would use mul alu flag
 *       
 *       TODO examine this, not expecting it to work but better to know for sure
 */
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

  Instr &header(std::string const &msg) { InstructionComment::header(msg);  return *this; }
  Instr &comment(std::string msg)       { InstructionComment::comment(msg); return *this; }

  bool is_branch() const;
  void set_cond_tag(AssignCond cond);
  void set_push_tag(SetCond set_cond);

  std::string dump() const; 
  std::string mnemonic(bool with_comments = false) const;
  uint64_t code() const;
  static std::string dump(uint64_t in_code);
  static std::string mnemonic(uint64_t in_code);
  static std::string mnemonics(std::vector<uint64_t> in_code);

  operator uint64_t() const { return code(); }

  void set_branch_condition(V3DLib::BranchCond src_cond);

  bool add_nop()    const { return alu.add.op == V3D_QPU_A_NOP; }
  bool mul_nop()    const { return alu.mul.op == V3D_QPU_M_NOP; }
  bool add_nocond() const { return flags.ac == V3D_QPU_COND_NONE; }
  bool mul_nocond() const { return flags.mc == V3D_QPU_COND_NONE; }


  static bool compare_codes(uint64_t code1, uint64_t code2);

  // TODO: figure out how to move these to Mnemonics
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

  void alu_mul_set(Location const &dst, Location const &a, Location const &b); 
  void alu_mul_set(Location const &dst, Location const &a, SmallImm const &b); 
  void alu_mul_set(Location const &dst, SmallImm const &a, Location const &b); 

  bool alu_mul_set(V3DLib::ALUInstruction const &alu, std::unique_ptr<Location> dst);

protected:
  static uint64_t const NOP;

  void init(uint64_t in_code);
  void set_branch_condition(v3d_qpu_branch_cond cond);

private:
  void init_ver() const;
  bool raddr_a_is_safe(Location const &loc, bool check_for_mul_b = false) const;
  bool raddr_b_is_safe(Location const &loc, bool check_for_mul_b = false) const;
  std::string pretty_instr() const;
};


bool can_convert_to_mul_instruction(ALUInstruction const &add_ialu);

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


/**
 * Why this has suddenly become necessary is beyond me.
 *
 * also, it looks recursive to me - just going with the flow, some C++ shit is arcane.
 */
inline Instructions &operator<<(Instructions &lhs, instr::Instr const &rhs) {
  lhs << rhs;
  return lhs;
}

}  // v3d
}  // V3DLib

#endif  // _V3DLIB_V3D_INSTR_INSTR_H
