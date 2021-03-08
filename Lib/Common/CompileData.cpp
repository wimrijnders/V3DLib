#include "CompileData.h"
#include "Support/basics.h"

namespace V3DLib {

CompileData compile_data;


void CompileData::clear() {
  liveness_dump.clear();
  target_code_before_regalloc.clear();
  allocated_registers_dump.clear();
  num_accs_introduced = 0;
}


void CompileData::allocated_registers(std::vector<Reg> const &allocated_regs) {
  std::string ret;

  for (int i = 0; i < (int) allocated_regs.size(); i++) {
    ret << i << ": " << allocated_regs[i].dump() << "\n";
  }

  allocated_registers_dump = ret; 
}

}  // namespace V3DLib
