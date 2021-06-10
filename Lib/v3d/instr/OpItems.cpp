#include "OpItems.h"
#include <vector>
#include "Support/basics.h"

namespace {

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
  { ALUOp::A_FADD,   V3D_QPU_A_FADD,  V3D_QPU_M_ADD },
  { ALUOp::A_FSUB,   V3D_QPU_A_FSUB,  V3D_QPU_M_SUB },
  { ALUOp::A_FtoI,   V3D_QPU_A_FTOIN  },
  { ALUOp::A_ItoF,   V3D_QPU_A_ITOF   },
  { ALUOp::A_ADD,    V3D_QPU_A_ADD    },
  { ALUOp::A_SUB,    V3D_QPU_A_SUB    },
  { ALUOp::A_SHR,    V3D_QPU_A_SHR    },
  { ALUOp::A_ASR,    V3D_QPU_A_ASR    },
  { ALUOp::A_SHL,    V3D_QPU_A_SHL    },
  { ALUOp::A_MIN,    V3D_QPU_A_MIN    },
  { ALUOp::A_MAX,    V3D_QPU_A_MAX    },
  { ALUOp::A_BAND,   V3D_QPU_A_AND    },
  { ALUOp::A_BOR,    V3D_QPU_A_OR     },
  { ALUOp::A_BXOR,   V3D_QPU_A_XOR    },
  { ALUOp::M_FMUL,   false,           V3D_QPU_M_FMUL },
  { ALUOp::M_MUL24,  false,           V3D_QPU_M_SMUL24 },
  { ALUOp::M_ROTATE, false,           V3D_QPU_M_MOV },     // Special case: it's a mul alu mov with sig.rotate set
  { ALUOp::A_TIDX,   V3D_QPU_A_TIDX   },
  { ALUOp::A_EIDX,   V3D_QPU_A_EIDX   },
  { ALUOp::A_FFLOOR, V3D_QPU_A_FFLOOR }
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
    std::string msg = "Could not find item for ";
    msg  << "op: " << op;
    assertq(false, msg, true);
    return nullptr;
  }

  return &op_items[index];
}

}  // anon namespace


namespace V3DLib {
namespace v3d {
namespace instr {

bool OpItems::uses_add_alu(V3DLib::Instr const &instr) {
  if (instr.tag != ALU) return false;
  auto op = instr.ALU.op.value();
  op_item const *item = op_items_find_by_op(op);
  assert(item != nullptr);

  if (!item->has_add_op) return false;
/*
  if (item->has_mul_op) {
    std::string msg;
    msg << "uses_add_alu(): target lang alu op '" << op << "' also has mul translation.";
    warning(msg);
  }
*/

  return true;
}


bool OpItems::uses_mul_alu(V3DLib::Instr const &instr) {
  if (instr.tag != ALU) return false;
  auto op = instr.ALU.op.value();
  op_item const *item = op_items_find_by_op(op);
  assert(item != nullptr);

  if (!item->has_mul_op) return false;

/*
  if (item->has_add_op) {
    std::string msg;
    msg << "uses_mul_alu(): target lang alu op '" << op << "' also has add translation.";
    warning(msg);
  }
*/

  return true;
}


bool OpItems::get_add_op(ALUInstruction const &add_alu, v3d_qpu_add_op &dst ) {
  auto op = add_alu.op.value();
  op_item const *item = op_items_find_by_op(op);
  assert(item != nullptr);

  if (!item->has_add_op) return false;
  dst = item->add_op;
  return true;
}


bool OpItems::get_mul_op(ALUInstruction const &add_alu, v3d_qpu_mul_op &dst ) {
  auto op = add_alu.op.value();
  op_item const *item = op_items_find_by_op(op);
  assert(item != nullptr);

  if (!item->has_mul_op) return false;
  dst = item->mul_op;
  return true;
}


/**
 * Combination only possible if instructions not both add ALU or both mul ALU
 */
bool OpItems::valid_combine_pair(V3DLib::Instr const &instr, V3DLib::Instr const &next_instr, bool &do_converse) {
  if (uses_add_alu(instr) && uses_mul_alu(next_instr)) {
    do_converse = false;
    return true;
  }

  if (uses_mul_alu(instr) && uses_add_alu(next_instr)) {
    do_converse = true;
    return true;
  }

  return false;
}

}  // namespace instr
}  // namespace v3d
}  // namespace V3DLib
