#include "OpItems.h"
#include <vector>
#include "Support/debug.h"

namespace V3DLib {
namespace {

std::vector<OpItem> m_list = {
  {SIN, "sin", true, ALUOp::A_FSIN}  // v3d-specific
};

}  // anon namespace

///////////////////////////////////////////////////////////////////////////////
// Class OpItem
///////////////////////////////////////////////////////////////////////////////

OpItem::OpItem(
  OpId in_tag,
  char const *in_str,
  bool in_is_function,
  ALUOp::Enum in_aluop_float
) :
  tag(in_tag),
  str(in_str),
  is_function(in_is_function),
  m_aluop_float(in_aluop_float)
{}


bool OpItem::is_unary() const {
  if (is_function) return true;
  assertq(false, "Further specify is_unary()");
  return false;
}


ALUOp::Enum OpItem::aluop_float() const {
  assertq(m_aluop_float != ALUOp::NONE, "ALU Op not defined for OpItem");
  return m_aluop_float;
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

}  // namespace V3DLib

