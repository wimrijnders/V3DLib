#include "Op.h"
#include <vector>
#include "Support/basics.h"
#include "OpItems.h"

namespace V3DLib {

Op::Op(Op const &rhs) : op(rhs.op), type(rhs.type), m_item(OpItems::get(rhs.op)) {}
Op::Op(OpId in_op, BaseType in_type) : op(in_op), type(in_type), m_item(OpItems::get(in_op)) {}

bool Op::isUnary()       const { return (m_item.num_params() == 1); }
std::string Op::dump()   const { return m_item.dump(); }
ALUOp::Enum Op::opcode() const { return OpItems::opcode(*this); }

std::string Op::disp(std::string const &lhs, std::string const &rhs) const { return m_item.disp(lhs, rhs); }

}  // namespace V3DLib
