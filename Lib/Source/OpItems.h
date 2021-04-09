#ifndef _V3DLIB_SOURCE_OPITEMS_H_
#define _V3DLIB_SOURCE_OPITEMS_H_
#include "Op.h"
#include "Target/instr/ALUOp.h"

namespace V3DLib {

struct OpItem {
  OpItem(OpId in_tag, char const *in_str, bool in_is_function, ALUOp::Enum in_aluop_float);

  OpId tag;
  char const *str;
  bool is_function;

  bool is_unary() const;
  ALUOp::Enum aluop_float() const;

private:
  ALUOp::Enum m_aluop_float;
};


class OpItems {
public:
  static OpItem const *find(OpId id);
};


}  // namespace V3DLib

#endif  // _V3DLIB_SOURCE_OPITEMS_H_
