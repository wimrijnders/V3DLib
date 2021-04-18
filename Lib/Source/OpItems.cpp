#include "OpItems.h"
#include <vector>
#include "Support/basics.h"
#include "Support/Platform.h"

namespace V3DLib {
namespace {

std::vector<OpItem> m_list = {
  {ADD,    "+",        false, ALUOp::A_FADD,   ALUOp::A_ADD},
  {SUB,    "-",        false, ALUOp::A_FSUB,   ALUOp::A_SUB},
  {MUL,    "*",        false, ALUOp::M_FMUL,   ALUOp::M_MUL24},
  {MIN,    " min ",    false, ALUOp::A_FMIN,   ALUOp::A_MIN},
  {MAX,    " max ",    false, ALUOp::A_FMAX,   ALUOp::A_MAX},
  {ItoF,   "(Float) ", false, ALUOp::A_ItoF,   ALUOp::NONE,    false, 1},
  {FtoI,   "(Int) ",   false, ALUOp::NONE,     ALUOp::A_FtoI,  false, 1},
  {ROTATE, " rotate ", false, ALUOp::M_ROTATE, ALUOp::M_ROTATE},
  {SHL,    " << ",     false, ALUOp::NONE,     ALUOp::A_SHL},
  {SHR,    " >> ",     false, ALUOp::NONE,     ALUOp::A_ASR},
  {USHR,   " _>> ",    false, ALUOp::NONE,     ALUOp::A_SHR},
  {ROR,    " ror ",    false, ALUOp::NONE,     ALUOp::A_ROR},
  {BAND,   " & ",      false, ALUOp::NONE,     ALUOp::A_BAND},
  {BOR,    " | ",      false, ALUOp::NONE,     ALUOp::A_BOR},
  {BXOR,   " ^ ",      false, ALUOp::NONE,     ALUOp::A_BXOR},
  {BNOT,   "~",        false, ALUOp::NONE,     ALUOp::A_BNOT, false, 1},

  // v3d-specific
  {FFLOOR, "ffloor",    true, ALUOp::A_FFLOOR, ALUOp::NONE,   true},
  {SIN,    "sin",       true, ALUOp::A_FSIN,   ALUOp::NONE,   true},  // also SFU function
  {TIDX,   "tidx",     false, ALUOp::NONE,     ALUOp::A_TIDX, true, 0},
  {EIDX,   "eidx",     false, ALUOp::NONE,     ALUOp::A_EIDX, true, 0},

  // SFU functions
  {RECIP,     "recip",     true, ALUOp::NONE,     ALUOp::NONE},
  {RECIPSQRT, "recipsqrt", true, ALUOp::NONE,     ALUOp::NONE},
  {EXP,       "exp",       true, ALUOp::NONE,     ALUOp::NONE},
  {LOG,       "log",       true, ALUOp::NONE,     ALUOp::NONE}
};

}  // anon namespace

///////////////////////////////////////////////////////////////////////////////
// Class OpItem
///////////////////////////////////////////////////////////////////////////////

OpItem::OpItem(
  OpId in_tag,
  char const *in_str,
  bool in_is_function,
  ALUOp::Enum in_aluop_float,
  ALUOp::Enum in_aluop_int,
  bool in_v3d_specific,
  int in_num_params
) :
  tag(in_tag),
  str(in_str),
  is_function(in_is_function),
  m_aluop_float(in_aluop_float),
  m_aluop_int(in_aluop_int),
  m_num_params(in_is_function?1:in_num_params),
  m_v3d_specific(in_v3d_specific)
{}


int OpItem::num_params() const {
  assertq(!is_function || m_num_params == 1, "Function op should always have 1 parameter");
  return m_num_params;
}


ALUOp::Enum OpItem::aluop_float() const {
  assertq(m_aluop_float != ALUOp::NONE, "ALU Op float not defined for OpItem");
  return m_aluop_float;
}


ALUOp::Enum OpItem::aluop_int() const {
  assertq(m_aluop_int != ALUOp::NONE, "ALU Op int not defined for OpItem");
  return m_aluop_int;
}


std::string OpItem::disp(std::string const &lhs, std::string const &rhs) const {
  std::string ret;

  if (num_params() == 0) {
    ret << str << "()";
  } else if (is_function) {
    assert(!lhs.empty());
    ret << str << "(" << lhs << ")";
  } else if (num_params() == 1) {
    assert(!lhs.empty());
    ret << "(" << str << lhs << ")";
  } else {
    assert(num_params() == 2);
    assert(!lhs.empty());
    assert(!rhs.empty());
    ret << "(" << lhs << str << rhs <<  ")";
  }

  return ret;
}


std::string OpItem::dump() const {
  std::string ret;
  ret << "Op " << tag << " (" << str << ")";
  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Class OpItems
///////////////////////////////////////////////////////////////////////////////

OpItem const *OpItems::find(OpId id) {
  for (auto &it : m_list) {
    if (it.tag == id) return &it;
  }
  return nullptr;
}


OpItem const &OpItems::get(OpId id) {
  OpItem const *item = find(id);
  assert(item != nullptr);
  return *item;
}


/**
 * Translate source operator to target opcode
 */
ALUOp::Enum OpItems::opcode(Op const &op) {
  auto const *item = OpItems::find(op.op);

  if (item == nullptr) {
    if (op.type == BaseType::FLOAT) {
      assertq(false, "opcode(): Unhandled op for float", true);
    } else {
      assertq(false, "opcode(): Unhandled op for int", true);
    }
    return ALUOp::NOP;
  }

  if (item->v3d_specific()) {
    if (Platform::compiling_for_vc4()) {
      std::string msg;
      msg << "opcode(): " << item->dump() << " is only for v3d";
      assertq(false, msg, true);
    }
  }

  if (op.type == BaseType::FLOAT) {
    return item->aluop_float();
  } else {
    return item->aluop_int();
  }
}

}  // namespace V3DLib

