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

using V3DLib::ALUOp;

struct op_item {
  op_item(ALUOp::Enum in_op, v3d_qpu_add_op in_add_op) :
    op(in_op),
    has_add_op(true),
    add_op(in_add_op)
  {}

  op_item(ALUOp::Enum in_op, bool in_add_op, v3d_qpu_mul_op in_mul_op) :
    op(in_op),
    has_mul_op(true),
    mul_op(in_mul_op)
  {
    assert(!in_add_op);
  }

  op_item(ALUOp::Enum in_op, v3d_qpu_add_op in_add_op, v3d_qpu_mul_op in_mul_op) :
    op(in_op),
    has_add_op(true),
    add_op(in_add_op),
    has_mul_op(true),
    mul_op(in_mul_op)
  {}

  ALUOp::Enum op;
  bool has_add_op       = false;
  v3d_qpu_add_op add_op = V3D_QPU_A_NOP;
  bool has_mul_op       = false;
  v3d_qpu_mul_op mul_op = V3D_QPU_M_NOP;
};


std::vector<op_item> op_items = {
  { ALUOp::A_FADD,   V3D_QPU_A_FADD, V3D_QPU_M_ADD },
  { ALUOp::A_FSUB,   V3D_QPU_A_FSUB, V3D_QPU_M_SUB },
  { ALUOp::A_FtoI,   V3D_QPU_A_FTOIN },
  { ALUOp::A_ItoF,   V3D_QPU_A_ITOF  },
  { ALUOp::A_ADD,    V3D_QPU_A_ADD   },
  { ALUOp::A_SUB,    V3D_QPU_A_SUB   },
  { ALUOp::A_SHR,    V3D_QPU_A_SHR   },
  { ALUOp::A_ASR,    V3D_QPU_A_ASR   },
  { ALUOp::A_SHL,    V3D_QPU_A_SHL   },
  { ALUOp::A_MIN,    V3D_QPU_A_MIN   },
  { ALUOp::A_MAX,    V3D_QPU_A_MAX   },
  { ALUOp::A_BAND,   V3D_QPU_A_AND   },
  { ALUOp::A_BOR,    V3D_QPU_A_OR    },
  { ALUOp::A_BXOR,   V3D_QPU_A_XOR   },
  { ALUOp::M_FMUL,   false,          V3D_QPU_M_FMUL },
  { ALUOp::M_MUL24,  false,          V3D_QPU_M_SMUL24 },
  { ALUOp::M_ROTATE, false,          V3D_QPU_M_MOV }      // Special case: it's a mul alu mov with sig.rotate set
};


void op_items_check_sorted() {
  static bool checked = false;

  if (checked) return;

  bool did_first = false;
  ALUOp::Enum previous;

  for (auto const &item : op_items) {
    if (!did_first) {
      previous = item.op;
      did_first = true;
      continue;
    }

    assertq(previous < item.op, "op_items not sorted on (target) op");
    previous = item.op;
  }

  checked = true;
}


/**
 * Derived from (iterative version): https://iq.opengenus.org/binary-search-in-cpp/
 */
int op_items_binary_search(int left, int right, ALUOp::Enum needle) {
  while (left <= right) { 
    int middle = (left + right) / 2; 

    if (op_items[middle].op == needle) 
      return middle;  // found it

    // If element is greater, ignore left half 
    if (op_items[middle].op < needle) 
      left = middle + 1; 

    // If element is smaller, ignore right half 
    else
      right = middle - 1; 
  } 

  return -1; // element not found
}


op_item const *op_items_find_by_op(ALUOp::Enum op) {
  op_items_check_sorted();

  int index = op_items_binary_search(0, (int) op_items.size() - 1, op);

  if (index != -1) {
    return &op_items[index];
  }

/*
  for (auto const &item : op_items) {
    if (item.op == op) {
      return &item;
    }
  }
*/

  return nullptr;
}

}  // anon namespace


namespace V3DLib {
namespace v3d {

using ::operator<<;  // C++ weirdness; come on c++, get a grip.

namespace instr {

uint64_t const Instr::NOP = 0x3c003186bb800000;  // This is actually 'nop nop'


Instr::Instr(uint64_t in_code) {
  init(in_code);
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


/**
 * Convert conditions from Target source to v3d
 *
 * Incoming conditions are vc4 only, the conditions don't exist on v3d.
 * They therefore need to be translated.
 */
void Instr::set_branch_condition(V3DLib::BranchCond src_cond) {
  // TODO How to deal with:
  //
  //      instr.na0();
  //      instr.a0();

  if (src_cond.tag == COND_ALWAYS) {
    return;  // nothing to do
  } else if (src_cond.tag == COND_ALL) {
    switch (src_cond.flag) {
      case ZC:
      case NC:
        set_branch_condition(V3D_QPU_BRANCH_COND_ALLNA);
        break;
      case ZS:
      case NS:
        set_branch_condition(V3D_QPU_BRANCH_COND_ALLA);
        break;
      default:
        debug_break("Unknown branch condition under COND_ALL");  // Warn me if this happens
    }
  } else if (src_cond.tag == COND_ANY) {
    switch (src_cond.flag) {
      case ZC:
      case NC:
        set_branch_condition(V3D_QPU_BRANCH_COND_ANYNA);  // TODO: verify
        break;
      case ZS:
      case NS:
        set_branch_condition(V3D_QPU_BRANCH_COND_ANYA);   // TODO: verify
        break;
      default:
        debug_break("Unknown branch condition under COND_ANY");  // Warn me if this happens
    }
  } else {
    debug_break("Branch condition not COND_ALL or COND_ANY");  // Warn me if this happens
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
// Conditions branch instructions
///////////////////////////////////////////////////////////////////////////////

void Instr::set_branch_condition(v3d_qpu_branch_cond cond) {
  assert(is_branch());  // Branch instruction-specific
  branch.cond = cond;
}

///////////////////////////////////////////////////////////////////////////////
// End Conditions  branch instructions
///////////////////////////////////////////////////////////////////////////////


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
 * @param dst  output parameter, recieves equivalent instruction if found
 *
 * @return  true if equivalent found, false otherwise
 */
bool convert_to_mul_instruction(ALUInstruction const &add_alu, v3d_qpu_mul_op &dst ) {
  auto op = add_alu.op.value();

  if (op == ALUOp::A_BOR) {
    // Special case: OR with same inputs can be considered a MOV
    // Handles rf-registers and accumulators only, fails otherwise
    // (ie. case combined tmua tmud won't work).
    if ((add_alu.dest.tag <= ACC)
     && (add_alu.srcA == add_alu.srcB)
     && (add_alu.srcA.is_imm() || add_alu.srcA.reg().tag <= ACC)  // Verified this check is required, won't work with special registers
    ) {
      dst = V3D_QPU_M_MOV;  // _FMOV
      return true;
    }
  }


  // Handle general case
  op_item const *item = op_items_find_by_op(op);
  assert(item != nullptr);
  if (!item->has_mul_op) return false;
  dst = item->mul_op;
  return true;
}

}  // anon namespace


void Instr::alu_add_set_reg_a(RegOrImm const &reg) {
  if (reg.is_reg()) {
    assert(reg.reg().tag != NONE);
    auto src_b = encodeSrcReg(reg.reg());
    assert(src_b);
    alu_add_set_reg_a(*src_b);
  } else {
    assert(reg.is_imm());
    SmallImm imm(reg.imm().val);
    alu_add_set_imm_a(imm);
  }
}


void Instr::alu_mul_set_reg_a(RegOrImm const &reg) {
  if (reg.is_reg()) {
    assert(reg.reg().tag != NONE);
    auto src_b = encodeSrcReg(reg.reg());
    assert(src_b);
    alu_mul_set_reg_a(*src_b);
  } else {
    assert(reg.is_imm());
    SmallImm imm(reg.imm().val);
    alu_mul_set_imm_a(imm);
  }
}


void Instr::alu_add_set_reg_b(RegOrImm const &reg) {
  if (reg.is_reg()) {
    assert(reg.reg().tag != NONE);
    auto src_b = encodeSrcReg(reg.reg());
    assert(src_b);
    alu_add_set_reg_b(*src_b);
  } else {
    assert(reg.is_imm());
    SmallImm imm(reg.imm().val);
    alu_add_set_imm_b(imm);
  }
}


void Instr::alu_mul_set_reg_b(RegOrImm const &reg) {
  if (reg.is_reg()) {
    assert(reg.reg().tag != NONE);
    auto src_b = encodeSrcReg(reg.reg());
    assert(src_b);
    alu_mul_set_reg_b(*src_b);
  } else {
    assert(reg.is_imm());
    SmallImm imm(reg.imm().val);
    alu_mul_set_imm_b(imm);
  }
}


/**
 * TODO misnomer, can set mul alu as well. Rename
 */
bool Instr::alu_add_set(V3DLib::Instr const &src_instr) {
  assert(add_nop());
  assert(mul_nop());

  auto op = src_instr.ALU.op.value();
  auto dst = encodeDestReg(src_instr);
  assert(dst);

  auto reg_a = src_instr.ALU.srcA;
  auto reg_b = src_instr.ALU.srcB;

  op_item const *item = op_items_find_by_op(op);
  if (item == nullptr) {
    std::string msg = "Could not find item for ";
    msg  << "op: " << src_instr.ALU.op.value()
         << ", instr: " << src_instr.dump();
    assertq(false, msg);
    return false;
  }

  if (item->has_add_op) {
    alu.add.op = item->add_op;
    alu_add_set_dst(*dst);
    alu_add_set_reg_a(reg_a);
    alu_add_set_reg_b(reg_b);
    return true;
  }

  if (item->has_mul_op) {
    alu.mul.op = item->mul_op;
    alu_mul_set_dst(*dst);
    alu_mul_set_reg_a(reg_a);
    alu_mul_set_reg_b(reg_b);
    return true;
  }


  std::string msg = "Unknown conversion for src ";
  msg  << "op: " << src_instr.ALU.op.value()
       << ", instr: " << src_instr.dump();
  assertq(false, msg, true);
  //warning(msg);

  return false;
}


/**
 * @return true if mul instruction set, false otherwise
 */
bool Instr::alu_mul_set(V3DLib::Instr const &src_instr) {
  assert(mul_nop());
  auto const &alu = src_instr.ALU;
  auto dst = encodeDestReg(src_instr);
  assert(dst);

  v3d_qpu_mul_op mul_op;
  if (!convert_to_mul_instruction(alu, mul_op)) {
    return false;
  }

  auto reg_a = alu.srcA;
  auto reg_b = alu.srcB;

  this->alu.mul.op = mul_op;
  this->alu_mul_set_dst(*dst);
  this->alu_mul_set_reg_a(reg_a);
  this->alu_mul_set_reg_b(reg_b);
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

}  // instr


///////////////////////////////////////////////////////////////////////////////
// Class Instructions
//////////////////////////////////////////////////////////////////////////////

Instructions &Instructions::comment(std::string msg, bool to_front) {
  assert(!empty());

  if (to_front) {
    front().comment(msg);
  } else {
    back().comment(msg);
  }

  return *this;
}


/**
 * Use param as run condition for current instruction(s)
 */
void Instructions::set_cond_tag(AssignCond cond) {
  for (auto &instr : *this) {
    instr.set_cond_tag(cond);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Class OpItems
///////////////////////////////////////////////////////////////////////////////

bool OpItems::uses_add_alu(V3DLib::Instr const &instr) {
  if (instr.tag != ALU) return false;
  auto op = instr.ALU.op.value();
  op_item const *item = op_items_find_by_op(op);
  assert(item != nullptr);

  if (!item->has_add_op) return false;

  if (item->has_mul_op) {
    std::string msg;
    msg << "uses_add_alu(): target lang alu op '" << op << "' also has mul translation.";
    warning(msg);
  }

  return true;
}


bool OpItems::uses_mul_alu(V3DLib::Instr const &instr) {
  if (instr.tag != ALU) return false;
  auto op = instr.ALU.op.value();
  op_item const *item = op_items_find_by_op(op);
  assert(item != nullptr);

  if (!item->has_mul_op) return false;

  if (item->has_add_op) {
    std::string msg;
    msg << "uses_mul_alu(): target lang alu op '" << op << "' also has add translation.";
    warning(msg);
  }

  return true;
}


bool OpItems::can_use_mul_alu(V3DLib::Instr const &instr) {
  if (instr.tag != ALU) return false;
  auto op = instr.ALU.op.value();
  op_item const *item = op_items_find_by_op(op);
  assert(item != nullptr);

  return (item->has_mul_op);
}


/**
 * Combination only possible if instructions not both add ALU or both mul ALU
 */
bool OpItems::valid_combine_pair(V3DLib::Instr const &instr, V3DLib::Instr const &next_instr, bool &do_converse) {
  if (uses_add_alu(instr) && can_use_mul_alu(next_instr)) {
    do_converse = false;
    return true;
  }

  if (can_use_mul_alu(instr) && uses_add_alu(next_instr)) {
    do_converse = true;
    return true;
  }

  return false;
}

}  // v3d


}  // V3DLib
