#include "Op.h"
#include <vector>
#include "Support/debug.h"
#include "OpItems.h"

namespace V3DLib {

Op::Op(Op const &rhs) : op(rhs.op), type(rhs.type), m_item(OpItems::get(rhs.op)) {}
Op::Op(OpId in_op, BaseType in_type) : op(in_op), type(in_type), m_item(OpItems::get(in_op)) {}

const char *Op::to_string() const {
  OpItem const *item = OpItems::find(op);
  if (item != nullptr) {
    return item->str;
  }

  assertq(false, "Op::to_string(): unknown opcode", true);
  return nullptr;
}


bool Op::noParams() const {
  return (op == TIDX  || op == EIDX);
}


bool Op::isUnary() const {
  OpItem const *item = OpItems::find(op);
  assert(item != nullptr);
  return (item->num_params() == 1);
}


bool Op::isFunction() const {
  OpItem const *item = OpItems::find(op);
  assert(item != nullptr);
  return item->is_function;
}


std::string Op::dump() const {
  OpItem const *item = OpItems::find(op);
  if (item != nullptr) {
    return item->dump();
  } else {
    return "<Unknown Op>";
  }
}

}  // namespace V3DLib
