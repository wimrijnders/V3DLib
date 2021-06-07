///////////////////////////////////////////////////////////////////////////////
// Opcodes for v3d
//
///////////////////////////////////////////////////////////////////////////////
#include "Instr.h"
#include <cstdio>
#include <cstdlib>        // abs()
#include <bits/stdc++.h>  // swap()
#include "dump_instr.h"
#include "Support/basics.h"
#include "Mnemonics.h"

namespace {

struct v3d_device_info devinfo;  // NOTE: uninitialized struct, field 'ver' must be set! For asm/disasm OK

}  // anon namespace


namespace V3DLib {
namespace v3d {
namespace instr {

uint64_t const Instr::NOP = 0x3c003186bb800000;  // This is actually 'nop nop'


Instr::Instr(uint64_t in_code) {
  init(in_code);
}


/**
 * Initialize the add alu
 */
Instr::Instr(v3d_qpu_add_op op, Location const &dst, Location const &a, Location const &b) {
  init(NOP);
  alu_add_set(dst, a, b);
  alu.add.op = op;
}


/**
 * Initialize the add alu
 */
Instr::Instr(v3d_qpu_add_op op, Location const &dst, Location const &a, SmallImm const &b) {
  init(NOP);
  alu_add_set(dst, a, b);
  alu.add.op = op;
}


/**
 * Initialize the add alu
 */
Instr::Instr(v3d_qpu_add_op op, Location const &dst, SmallImm const &a, Location const &b) {
  init(NOP);
  alu_add_set(dst, a, b);
  alu.add.op = op;
}


/**
 * This only works if imma == immb (test here internally)
 * The syntax, however, allows this.
 */
Instr::Instr(v3d_qpu_add_op op, Location const &dst, SmallImm const &a, SmallImm const &b) {
  init(NOP);
  alu_add_set(dst, a, b);
  alu.add.op = op;
}


bool Instr::is_branch() const {
  return (type == V3D_QPU_INSTR_TYPE_BRANCH);
}

namespace {

v3d_qpu_cond translate_assign_cond(AssignCond cond) {
  assertq(cond.tag != AssignCond::Tag::NEVER, "Not expecting NEVER (yet)", true);

  v3d_qpu_cond tag_value = V3D_QPU_COND_NONE;

  if (cond.tag != AssignCond::Tag::FLAG) return tag_value;

  switch(cond.flag) {
    case ZS:
    case NS:
      // set ifa tag
      tag_value = V3D_QPU_COND_IFA;
      break;
    case ZC:
    case NC:
      // set ifna tag
      tag_value = V3D_QPU_COND_IFNA;
      break;
    default: break;
  }

  return tag_value;
}

} // anon namespace

/**
 * Set the condition tags during translation.
 *
 * Either add or mul alu condition tags are set here, both not allowed (intentionally too strict condition)
 */
void Instr::set_cond_tag(AssignCond cond) {
  assert(!is_branch());
  if (cond.is_always()) return;
  if (alu.add.op == V3D_QPU_A_NOP && alu.mul.op == V3D_QPU_M_NOP) return;  // Don't bother with a full nop instruction

  assertq(flags.ac == V3D_QPU_COND_NONE, "Not expecting add alu assign tag to be set");
  assertq(flags.mc == V3D_QPU_COND_NONE, "Not expecting mul alu assign tag to be set");
  assertq(cond.tag != AssignCond::Tag::NEVER, "Not expecting NEVER (yet)", true);
  assertq(cond.tag == AssignCond::Tag::FLAG,  "const.tag can only be FLAG here");  // The only remaining option

  v3d_qpu_cond tag_value = translate_assign_cond(cond);
  assert(tag_value != V3D_QPU_COND_NONE);

  assertq(!(alu.add.op != V3D_QPU_A_NOP && alu.mul.op != V3D_QPU_M_NOP),
    "Not expecting both add and mul alu to be used"); 

  if (alu.add.op != V3D_QPU_A_NOP) {
    flags.ac = tag_value;
  }

  if (alu.mul.op != V3D_QPU_M_NOP) {
    flags.mc = tag_value;
  }
}


void Instr::set_push_tag(SetCond set_cond) {
  if (set_cond.tag() == SetCond::NO_COND) return;
  assertq(flags.apf == V3D_QPU_PF_NONE, "Not expecting add alu push tag to be set");
  assertq(flags.mpf == V3D_QPU_PF_NONE, "Not expecting mul alu push tag to be set");
  assertq(set_cond.tag() == SetCond::Z || set_cond.tag() == SetCond::N, "Unhandled SetCond flag", true);

  v3d_qpu_pf tag_value;

  if (set_cond.tag() == SetCond::Z) {
    tag_value = V3D_QPU_PF_PUSHZ;
  } else {
    tag_value = V3D_QPU_PF_PUSHN;
  }

  assertq(!(alu.add.op != V3D_QPU_A_NOP && alu.mul.op != V3D_QPU_M_NOP),
    "Not expecting both add and mul alu to be pushed"); // Warn me if this happens, deal with it then

  if (alu.add.op != V3D_QPU_A_NOP) {
    flags.apf = tag_value;
  }

  if (alu.mul.op != V3D_QPU_M_NOP) {
    flags.mpf = tag_value;
  }
}


std::string Instr::dump() const {
  char buffer[10*1024];
  instr_dump(buffer, const_cast<Instr *>(this));
  return std::string(buffer);
}


std::string Instr::pretty_instr() const {
  std::string ret = instr_mnemonic(this);

  auto indent = [] (int size) -> std::string {
    const int TAB_POS = 42;  // tab position for signal in mnemonic
    size++;                  // Fudging to get position right
    std::string ret;

    while (size < TAB_POS) {
      ret << " ";
      size++;
    }

    return ret;
  };


  // Output rotate signal (not done in MESA)
  if (sig.rotate) {
    if (!is_branch()) {  // Assumption: rotate signal irrelevant for branch

      // Only two possibilities here: r5 or small imm (sig for small imm not set!)
      if (alu.mul.b == V3D_QPU_MUX_R5) {
        ret << ", r5";
      } else if (alu.mul.b == V3D_QPU_MUX_B) {
        ret << ", " << raddr_b;
      } else {
        assertq(false, "pretty_instr(): unexpected mux value for mul b for rotate", true);
      }

      ret << indent((int) ret.size()) << "; rot";
    }
  }

  return ret;
}


std::string Instr::mnemonic(bool with_comments) const {
  std::string ret;

  if (with_comments && !InstructionComment::header().empty()) {
    ret << emit_header();
  }

  std::string out = pretty_instr();
  ret << out;

  if (with_comments && !InstructionComment::comment().empty()) {
    ret << emit_comment((int) out.size());
  }

  return ret;
}


uint64_t Instr::code() const {
  init_ver();

  uint64_t repack = instr_pack(&devinfo, const_cast<Instr *>(this));
  return repack;
}


std::string Instr::dump(uint64_t in_code) {
  Instr instr(in_code);
  return instr.dump();
}


std::string Instr::mnemonic(uint64_t in_code) {
  Instr instr(in_code);
  return instr.mnemonic();
}


std::string Instr::mnemonics(std::vector<uint64_t> in_code) {
  std::string ret;

  for (int i = 0; i < (int) in_code.size(); i++) {
    ret << i << ": " << mnemonic(in_code[i]) << "\n";
  }

  return ret;
}


void Instr::init_ver() const {
  devinfo.ver = 42;  // only this needs to be set
}


void Instr::init(uint64_t in_code) {
  init_ver();

  raddr_a = 0;

  // These do not always get initialized in unpack
  sig_addr = 0;
  sig_magic = false;
  raddr_b = 0; // Not set for branch

  if (!instr_unpack(&devinfo, in_code, this)) {
    warning("Instr:init: call to instr_unpack failed.");
    return;
  }

  if (is_branch()) {
    if (!branch.ub) {
      // take over the value anyway
      branch.bdu = (v3d_qpu_branch_dest) ((in_code >> 15) & 0b111);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// End class Instr
///////////////////////////////////////////////////////////////////////////////

namespace {

#ifdef DEBUG
std::string binaryValue(uint64_t num) {
  const int size = sizeof(num)*8;
  std::string result; 

  for (int i = size -1; i >=0; i--) {
    bool val = ((num >> i) & 1) == 1;

    if (val) {
      result += '1';
    } else {
      result += '0';
    }

    if (i % 10 == 0) {
      result += '.';
    }
  }

  return result;
}
#endif

}  // anon namespace


// from mesa/src/broadcom/qpu/qpu_pack.c
#define QPU_MASK(high, low) ((((uint64_t)1<<((high)-(low)+1))-1)<<(low))
#define QPU_GET_FIELD(word, field) ((uint32_t)(((word)  & field ## _MASK) >> field ## _SHIFT))

#define VC5_QPU_BRANCH_MSFIGN_SHIFT         21
#define VC5_QPU_BRANCH_MSFIGN_MASK          QPU_MASK(22, 21)

#define VC5_QPU_BRANCH_COND_SHIFT           32
#define VC5_QPU_BRANCH_COND_MASK            QPU_MASK(34, 32)
// End from mesa/src/broadcom/qpu/qpu_pack.c



bool Instr::compare_codes(uint64_t code1, uint64_t code2) {
  if (code1 == code2) {
    return true;
  }

  // Here's the issue:
  // For the branch instruction, if field branch.ub != true, then field branch bdu is not used
  // and can have any value.
  // So for a truthful compare, in this special case the field needs to be ignored.
  // Determine if this is a branch
  auto is_branch = [] (uint64_t code) -> bool {
    uint64_t mul_op_mask = ((uint64_t) 0xb11111) << 58;
    bool is_mul_op = 0 != ((code & mul_op_mask) >> 58);

    uint64_t branch_sig_mask = ((uint64_t) 1) << 57;
    bool has_branch_sig = 0 != (code & branch_sig_mask) >> 57;

    return (!is_mul_op && has_branch_sig);
  };


  if (!is_branch(code1) || !is_branch(code2)) {
    return false;  // Not the special case we're looking for
  }

  // Hypothesis: non-usage of bdu depends on these two fields

  uint32_t cond   = QPU_GET_FIELD(code1, VC5_QPU_BRANCH_COND);
  uint32_t msfign = QPU_GET_FIELD(code1, VC5_QPU_BRANCH_MSFIGN);

  if (cond == 0) {
    //instr->branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  } else if (V3D_QPU_BRANCH_COND_A0 + (cond - 2) <= V3D_QPU_BRANCH_COND_ALLNA) {
    cond = V3D_QPU_BRANCH_COND_A0 + (cond - 2);
  }

  if (cond == 2 && msfign == V3D_QPU_MSFIGN_NONE) {
    // Zap out the bdu field and compare again
    uint64_t bdu_mask = ((uint64_t) 0b111) << 15;
    code1 = code1 & ~bdu_mask;
    code2 = code1 & ~bdu_mask;

    return code1 == code2;
  }

#ifdef DEBUG
  printf("compare_codes diff: %s\n", binaryValue(code1 ^ code2).c_str());
  printf("code1: %s\n", mnemonic(code1).c_str());
  printf("%s\n", dump(code1).c_str());
  printf("code2: %s\n", mnemonic(code2).c_str());
  printf("%s\n", dump(code2).c_str());

  breakpoint
#endif  // DEBUG

  return false;
}


///////////////////////////////////////////////////////////////////////////////
// Calls to set the mult part of the instruction
///////////////////////////////////////////////////////////////////////////////

void Instr::set_c(v3d_qpu_cond val) {
  if (m_doing_add) {
    flags.ac = val;
  } else {
    flags.mc = val;
  }
}


void Instr::set_uf(v3d_qpu_uf val) {
  if (m_doing_add) {
    flags.auf = val;
  } else {
    flags.muf = val;
  }
}


void Instr::set_pf(v3d_qpu_pf val) {
  if (m_doing_add) {
    flags.apf = val;
  } else {
    flags.mpf = val;
  }
}


Instr &Instr::pushc() { set_pf(V3D_QPU_PF_PUSHC); return *this; }
Instr &Instr::pushn() { set_pf(V3D_QPU_PF_PUSHN); return *this; }
Instr &Instr::pushz() { set_pf(V3D_QPU_PF_PUSHZ); return *this; }

Instr &Instr::norc()  { set_uf(V3D_QPU_UF_NORC);  return *this; }
Instr &Instr::nornc() { set_uf(V3D_QPU_UF_NORNC); return *this; }
Instr &Instr::norz()  { set_uf(V3D_QPU_UF_NORZ);  return *this; }
Instr &Instr::norn()  { set_uf(V3D_QPU_UF_NORN);  return *this; }
Instr &Instr::nornn() { set_uf(V3D_QPU_UF_NORNN); return *this; }
Instr &Instr::andn()  { set_uf(V3D_QPU_UF_ANDN);  return *this; }
Instr &Instr::andz()  { set_uf(V3D_QPU_UF_ANDZ);  return *this; }
Instr &Instr::andc()  { set_uf(V3D_QPU_UF_ANDC);  return *this; }
Instr &Instr::andnc() { set_uf(V3D_QPU_UF_ANDNC); return *this; }
Instr &Instr::andnn() { set_uf(V3D_QPU_UF_ANDNN); return *this; }

Instr &Instr::ifnb()  { set_c(V3D_QPU_COND_IFNB); return *this; }
Instr &Instr::ifb()   { set_c(V3D_QPU_COND_IFB);  return *this; }
Instr &Instr::ifna()  { set_c(V3D_QPU_COND_IFNA); return *this; }
Instr &Instr::ifa()   { set_c(V3D_QPU_COND_IFA);  return *this; }


Instr &Instr::ldtmu(Register const &reg) {
  sig.ldtmu = true;
  sig_addr  = reg.to_waddr(); 
  sig_magic = true;

  return *this;
}


Instr &Instr::thrsw()   { sig.thrsw   = true; return *this; }
Instr &Instr::ldvary()  { sig.ldvary  = true; return *this; }
Instr &Instr::ldunif()  { sig.ldunif  = true; return *this; }
Instr &Instr::ldunifa() { sig.ldunifa = true; return *this; }
Instr &Instr::ldvpm()   { sig.ldvpm   = true; return *this; }

Instr &Instr::ldunifarf(Location const &loc) {
  sig.ldunifarf = true;

  sig_magic = !loc.is_rf();
  sig_addr = loc.to_waddr();
  return *this;
}


Instr &Instr::ldunifrf(RFAddress const &loc) {
  sig.ldunifrf = true;

  sig_magic = !loc.is_rf();
  sig_addr = loc.to_waddr();
  return *this;
}


///////////////////////////////////////////////////////////////////////////////
// Conditions branch instructions
///////////////////////////////////////////////////////////////////////////////

Instr &Instr::set_branch_condition(v3d_qpu_branch_cond cond) {
  assert(is_branch());  // Branch instruction-specific
  branch.cond = cond;
  return *this;
}


Instr &Instr::a0()     { return set_branch_condition(V3D_QPU_BRANCH_COND_A0); }
Instr &Instr::na0()    { return set_branch_condition(V3D_QPU_BRANCH_COND_NA0); }
Instr &Instr::alla()   { return set_branch_condition(V3D_QPU_BRANCH_COND_ALLA); }
Instr &Instr::allna()  { return set_branch_condition(V3D_QPU_BRANCH_COND_ALLNA); }
Instr &Instr::anya()   { return set_branch_condition(V3D_QPU_BRANCH_COND_ANYA); }
Instr &Instr::anyaq()  { branch.msfign =  V3D_QPU_MSFIGN_Q; return anya(); }
Instr &Instr::anyap()  { branch.msfign =  V3D_QPU_MSFIGN_P; return anya(); }
Instr &Instr::anyna()  { return set_branch_condition(V3D_QPU_BRANCH_COND_ANYNA); }
Instr &Instr::anynaq() { branch.msfign =  V3D_QPU_MSFIGN_Q; return anyna(); }
Instr &Instr::anynap() { branch.msfign =  V3D_QPU_MSFIGN_P; return anyna(); }

///////////////////////////////////////////////////////////////////////////////
// End Conditions  branch instructions
///////////////////////////////////////////////////////////////////////////////


Instr &Instr::add(Location const &dst, Location const &srca, Location const &srcb) {
  m_doing_add = false;
  alu_mul_set(dst, srca, srcb); 
  alu.mul.op    = V3D_QPU_M_ADD;
  return *this;
}


Instr &Instr::sub(Location const &dst, Location const &srca, SmallImm const &immb) {
  m_doing_add = false;
  alu_mul_set(dst, srca, immb); 
  alu.mul.op    = V3D_QPU_M_SUB;
  return *this;
}


Instr &Instr::sub(Location const &loc1, Location const &loc2, Location const &loc3) {
  m_doing_add = false;
  alu_mul_set(loc1, loc2, loc3); 
  alu.mul.op    = V3D_QPU_M_SUB;
  return *this;
}


Instr &Instr::nop() {
  m_doing_add = false;
  // With normal usage, the mul-part is already nop
  return *this;
}


Instr &Instr::mov(Location const &dst, SmallImm const &imm) {
  m_doing_add = false;

  alu_mul_set_dst(dst);
  alu_mul_set_imm_a(imm);

  alu.mul.op    = V3D_QPU_M_MOV;
  alu.mul.b     = V3D_QPU_MUX_B;   // Apparently needs to be set also

  return *this;
}


Instr &Instr::fmov(Location const &dst,  SmallImm const &imma) {
  m_doing_add = false;
  alu_mul_set_dst(dst);
  alu_mul_set_imm_a(imma);

  alu.mul.op    = V3D_QPU_M_FMOV;  // TODO what's the difference with _MOV? Check
  alu.mul.b     = V3D_QPU_MUX_B;   // Apparently needs to be set also

  return *this;
}


/**
 * Can't consolidate this yet, required for special register vpm
 */
Instr &Instr::mov(uint8_t rf_addr, Register const &reg) {
  m_doing_add = false;

  alu.mul.op    = V3D_QPU_M_MOV;
  alu.mul.a     = reg.to_mux();
  alu.mul.b     = V3D_QPU_MUX_B;
  alu.mul.waddr = rf_addr;

  return *this;
}


Instr &Instr::mov(Location const &loc1, Location const &loc2) {
  m_doing_add = false;
  alu_mul_set(loc1, loc2, loc2); 

  alu.mul.op    = V3D_QPU_M_MOV;
  return *this;
}


Instr &Instr::fmul(Location const &loc1, Location const &loc2, Location const &loc3) {
  m_doing_add = false;
  alu_mul_set(loc1, loc2, loc3);

  alu.mul.op    = V3D_QPU_M_FMUL;
  return *this;
}


Instr &Instr::fmul(Location const &loc1, SmallImm imm2, Location const &loc3) {
  alu_mul_set(loc1, imm2,  loc3);
  alu.mul.op = V3D_QPU_M_FMUL;
  return *this;
}


Instr &Instr::fmul(Location const &loc1, Location const &loc2, SmallImm const &imm3) {
  alu_mul_set(loc1, loc2,  imm3);
  alu.mul.op = V3D_QPU_M_FMUL;
  return *this;
}


Instr &Instr::smul24(Location const &dst, Location const &loca, Location const &locb) {
  m_doing_add = false;
  alu_mul_set(dst, loca, locb);

  alu.mul.op    = V3D_QPU_M_SMUL24;
  return *this;
}


/**
 * NOTE: Added this one myself, not sure if correct
 * TODO verify correctness
 */
Instr &Instr::smul24(Location const &dst, SmallImm const &imma, Location const &locb) {
  m_doing_add = false;
  alu_mul_set(dst, imma, locb);

  alu.mul.op    = V3D_QPU_M_SMUL24;
  return *this;
}


/**
 * TODO verify correctness
 */
Instr &Instr::smul24(Location const &dst, Location const &loca, SmallImm const &immb) {
  m_doing_add = false;
  alu_mul_set(dst, loca, immb);

  alu.mul.op    = V3D_QPU_M_SMUL24;
  return *this;
}


Instr &Instr::vfmul(Location const &rf_addr1, Register const &reg2, Register const &reg3) {
  m_doing_add = false;
  alu_mul_set(rf_addr1, reg2, reg3);

  alu.mul.op = V3D_QPU_M_VFMUL;
  return *this;
}


void Instr::alu_add_set_dst(Location const &dst) {
  if (dst.is_rf()) {
    alu.add.magic_write = false; // selects address in register file
  } else {
    alu.add.magic_write = true;  // selects register
  }

  alu.add.waddr = dst.to_waddr();
  alu.add.output_pack = dst.output_pack();
}


void Instr::alu_add_set_reg_a(Location const &loc) {
  if (loc.is_rf()) {
    raddr_a = loc.to_waddr();
    alu.add.a = V3D_QPU_MUX_A;
  } else {
    alu.add.a = loc.to_mux();
  }

  alu.add.a_unpack = loc.input_unpack();
}


void Instr::alu_add_set_reg_b(Location const &loc) {
  if (loc.is_rf()) {
    if (!sig.small_imm) {
      if (alu.add.a == V3D_QPU_MUX_A) {
        // raddr_a already taken

        if (raddr_a == loc.to_waddr()) {
          // If it's the same, reuse
          alu.add.b        = V3D_QPU_MUX_A;
        } else {
          // use b instead
          raddr_b          = loc.to_waddr(); 
          alu.add.b        = V3D_QPU_MUX_B;
        }
      } else {
        raddr_a          = loc.to_waddr(); 
        alu.add.b        = V3D_QPU_MUX_A;
      }
    } else {
      // raddr_b contains a small imm, do raddr_a instead
      assert(alu.add.a == V3D_QPU_MUX_B);

      raddr_a          = loc.to_waddr(); 
      alu.add.b        = V3D_QPU_MUX_A;
    }
  } else {
    alu.add.b        = loc.to_mux();
  }

  alu.add.b_unpack = loc.input_unpack();
}


/**
 * Set the immediate value for an operation
 *
 * The immediate value is always set in raddr_b.
 * Multiple immediate operands are allowed in an instruction only if they are the same value
 */
void Instr::alu_set_imm(SmallImm const &imm) {
  if (sig.small_imm == true) {
    if (raddr_b != imm.to_raddr()) {
      fatal("Multiple immediate values in an operation only allowed if they are the same value");
    }
  } else {
    // All is well
    sig.small_imm = true; 
    raddr_b       = imm.to_raddr(); 
  }
}


void Instr::alu_add_set_imm_a(SmallImm const &imm) {
  alu_set_imm(imm);
  alu.add.a        = V3D_QPU_MUX_B;
  alu.add.a_unpack = imm.input_unpack();
}


void Instr::alu_add_set_imm_b(SmallImm const &imm) {
  alu_set_imm(imm);
  alu.add.b        = V3D_QPU_MUX_B;
  alu.add.b_unpack = imm.input_unpack();
}


/**
 * Copied from alu_add_set_imm_a(), not sure about this
 * TODO verify in some way
 */
void Instr::alu_mul_set_imm_a(SmallImm const &imm) {
  alu_set_imm(imm);
  alu.mul.a        = V3D_QPU_MUX_B;
  alu.mul.a_unpack = imm.input_unpack();
}


void Instr::alu_mul_set_imm_b(SmallImm const &imm) {
  alu_set_imm(imm);
  alu.mul.b     = V3D_QPU_MUX_B;
  alu.mul.b_unpack = imm.input_unpack();
}


void Instr::alu_add_set(Location const &dst, Location const &srca, Location const &srcb) {
  alu_add_set_dst(dst);
  alu_add_set_reg_a(srca);
  alu_add_set_reg_b(srcb);
}


void Instr::alu_add_set(Location const &dst, SmallImm const &imma, Location const &srcb) {
  alu_add_set_dst(dst);
  alu_add_set_imm_a(imma);
  alu_add_set_reg_b(srcb);
}


void Instr::alu_add_set(Location const &dst, Location const &srca, SmallImm const &immb) {
  alu_add_set_dst(dst);
  alu_add_set_reg_a(srca);
  alu_add_set_imm_b(immb);
}


void Instr::alu_add_set(Location const &dst, SmallImm const &imma, SmallImm const &immb) {
  alu_add_set_dst(dst);
  alu_add_set_imm_a(imma);
  alu_add_set_imm_b(immb);
}


void Instr::alu_mul_set_dst(Location const &dst) {
  if (dst.is_rf()) {
    alu.mul.magic_write = false; // selects address in register file
  } else {
    alu.mul.magic_write = true;  // selects register
  }

  alu.mul.waddr = dst.to_waddr();
  alu.mul.output_pack = dst.output_pack();
}


bool Instr::raddr_a_is_safe(Location const &loc, bool check_for_mul_b) const {
  // Is raddr_a in use by add alu?
  bool raddr_a_in_use = (alu.add.a == V3D_QPU_MUX_A) || (alu.add.b == V3D_QPU_MUX_A);
  if (check_for_mul_b) raddr_a_in_use = raddr_a_in_use || (alu.mul.a == V3D_QPU_MUX_A);

  bool raddr_a_same = (raddr_a == loc.to_waddr());

  return (!raddr_a_in_use || raddr_a_same);
}


bool Instr::raddr_b_is_safe(Location const &loc, bool check_for_mul_b) const {
  // Is raddr_a in use by add alu?
  bool raddr_a_in_use = (alu.add.a == V3D_QPU_MUX_B) || (alu.add.b == V3D_QPU_MUX_B);
  if (check_for_mul_b) raddr_a_in_use = raddr_a_in_use || (alu.mul.a == V3D_QPU_MUX_B);

  bool raddr_b_same = (raddr_b == loc.to_waddr());

  return (!raddr_a_in_use || raddr_b_same);
}


void Instr::alu_mul_set_reg_a(Location const &loc) {
  if (!loc.is_rf()) {
    // loc is a register
    alu.mul.a = loc.to_mux();
  } else if (raddr_a_is_safe(loc, true)) {
    raddr_a          = loc.to_waddr(); 
    alu.mul.a        = V3D_QPU_MUX_A;
  } else if (raddr_b_is_safe(loc, true)) {
    raddr_b   = loc.to_waddr(); 
    alu.mul.a = V3D_QPU_MUX_B;
  } else {
    debug_break("alu_add_set_reg_a(): raddr_a and raddr_b both in use");
  }

  alu.mul.a_unpack = loc.input_unpack();
}


void Instr::alu_mul_set_reg_b(Location const &loc) {
  if (!loc.is_rf()) {
    // loc is a register
    alu.mul.b = loc.to_mux();
  } else if (raddr_a_is_safe(loc, true)) {
    raddr_a          = loc.to_waddr(); 
    alu.mul.b        = V3D_QPU_MUX_A;
  } else if (raddr_b_is_safe(loc, true)) {
    raddr_b   = loc.to_waddr(); 
    alu.mul.b = V3D_QPU_MUX_B;
  } else {
    debug_break("alu_add_set_reg_b(): raddr_a and raddr_b both in use");
  }

  alu.mul.b_unpack = loc.input_unpack();
}


void Instr::alu_mul_set(Location const &dst, Location const &a, Location const &b) {
  alu_mul_set_dst(dst);
  alu_mul_set_reg_a(a);
  alu_mul_set_reg_b(b);
}


void Instr::alu_mul_set(Location const &dst, Location const &a, SmallImm const &b) {
  alu_mul_set_dst(dst);
  alu_mul_set_reg_a(a);
  alu_mul_set_imm_b(b);
}


void Instr::alu_mul_set(Location const &dst, SmallImm const &a, Location const &b) {
  alu_mul_set_dst(dst);
  alu_mul_set_imm_a(a);
  alu_mul_set_reg_b(b);
}


namespace {

/**
 * Get the v3d mul equivalent of a target lang add alu instruction.
 *
 * @param dst  output parameter, recieves equivalent insruction if found
 *
 * @return  true if equivalent found, false otherwise
 */
bool convert_to_mul_instruction(ALUInstruction const &add_alu, v3d_qpu_mul_op &dst ) {
  bool ret = true;

  switch(add_alu.op.value()) {
    case ALUOp::A_ADD:   dst = V3D_QPU_M_ADD;    break;
    case ALUOp::A_SUB:   dst = V3D_QPU_M_SUB;    break;
    case ALUOp::M_FMUL:  dst = V3D_QPU_M_FMUL;   break;
    case ALUOp::M_MUL24: dst = V3D_QPU_M_SMUL24; break;

    // NOT WORKING - apparently, I don't properly understand what MOV/FMOV does.
    //               compiles, but kernel output is wrong.
    //
    // Special case: OR with same inputs can be considered a MOV
    // Handles rf-registers and accumulators only, might be too strict
    // (eg. case cobined tmua tmud, perhaps possible?)
    case ALUOp::A_BOR:
      if ((add_alu.srcA == add_alu.srcB)
       && (add_alu.dest.tag <= ACC)
       && (add_alu.srcA.is_reg() && add_alu.srcA.reg().tag <= ACC)) {
        //breakpoint
        dst = V3D_QPU_M_MOV;  // _FMOV
      } else {
        ret = false;
      }
    break;

    default: ret = false; break;
  }

  return ret;
}

}  // anon namespace


bool can_convert_to_mul_instruction(ALUInstruction const &add_alu) {
  v3d_qpu_mul_op dst;
  return convert_to_mul_instruction(add_alu, dst);
}


/**
 * @return true if mul instruction set, false otherwise
 */
bool Instr::alu_mul_set(V3DLib::ALUInstruction const &alu, std::unique_ptr<Location> dst) {
  assert(dst);

  v3d_qpu_mul_op mul_op;
  if (!convert_to_mul_instruction(alu, mul_op)) {
    return false;
  }

  auto reg_a = alu.srcA;
  auto src_a = encodeSrcReg(reg_a.reg());
  assert(src_a);

  auto reg_b = alu.srcB;
  std::unique_ptr<Location> src_b;
  if (reg_b.is_reg()) {
    src_b = encodeSrcReg(reg_b.reg());
  }

  if (src_a && src_b) {
    alu_mul_set(*dst, *src_a, *src_b);
  } else if (src_a && reg_b.is_imm()) {
    SmallImm imm_b(reg_b.imm().val);
    alu_mul_set(*dst, *src_a, imm_b);
  } else {
    assert(false);
  }

  this->alu.mul.op = mul_op;
  flags.mc = translate_assign_cond(alu.cond);

  // TODO shouldn't push tag be done as well? Check
  // Normally set with set_push_tag()
  //this->alu.mul.m_setCond = alu.m_setCond;

  //std::cout << "alu_mul_set(ALU) result: " << mnemonic(true) << std::endl;
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Label support
//////////////////////////////////////////////////////////////////////////////

void Instr::label(int val) {
  assert(val >= 0);
  m_label = val;
}


int Instr::branch_label() const {
  assert(is_branch_label());
  return m_label;
}


bool Instr::is_branch_label() const {
  return type == V3D_QPU_INSTR_TYPE_BRANCH && m_label >= 0;
}


void Instr::label_to_target(int offset) {
  assert(!m_is_label);
  assert(0 <= branch_label());  // only do this to a branch instruction with a label
  assert(branch.offset == 0);   // Shouldn't have been set already

  // branch needs 4 delay slots before executing, hence the 4
  // This means that 3 more instructions will execute after the loop before jumping
  branch.offset = (unsigned) 8*(offset - 4);

  m_label = -1;
}


/**
 * Rotate for mul alu.
 *
 * Rotate only works via the mul ALU.
 *
 * See notes in header comment of rotate overload for add alu above.
 */
Instr &Instr::rotate(Location const &dst, Location const &a, SmallImm const &b) {
  assertq(dst.to_mux()  == V3D_QPU_MUX_R1, "rotate dest can only be r1");
  assertq(a.to_mux() == V3D_QPU_MUX_R0,    "rotate src a can only be r0", true);
  assertq(-15 <= b.val() && b.val() < 16,  "rotate: smallimm must be in proper range");

  m_doing_add = false;

  alu_mul_set(r1, r0, b);

  if (b.val() != 0) {  // Don't bother rotating if there is no rotate
    sig.rotate = true;
  }
  sig.small_imm = false;      // Should *not* be set for rotate
  alu.mul.op = V3D_QPU_M_MOV;

  return *this;
}


/**
 * Rotate for mul alu.
 *
 * See notes in header comment of rotate overload for add alu above.
 */
Instr &Instr::rotate(Location const &dst, Location const &a, Location const &b) {
  assertq(dst.to_mux()  == V3D_QPU_MUX_R1, "rotate dest can only be r1");
  assertq(a.to_mux() == V3D_QPU_MUX_R0,    "rotate src a can only be r0");
  assertq(b.to_mux() == V3D_QPU_MUX_R5,    "rotate src b can only be r5");
  // TODO: check value r5 within range -15,15 inclusive, possible?

  m_doing_add = false;

  alu_mul_set(r1, r0, r5);
  sig.rotate = true;
  alu.mul.op = V3D_QPU_M_MOV;

  return *this;
}

}  // instr
}  // v3d


}  // V3DLib
