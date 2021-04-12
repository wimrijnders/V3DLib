#include "CompileData.h"
#include "Support/basics.h"

namespace V3DLib {

//using ::operator<<;  // C++ weirdness

CompileData compile_data;

std::string CompileData::dump() const {
  std::string ret;

  ret << title("Liveness dump")
      << liveness_dump
      << title("Reg usage dump")
      << reg_usage_dump
      << title("Allocated registers to variables")
      << allocated_registers_dump;

  if (!target_code_before_optimization.empty()) {
    ret << title("Target code before optimization")
        << target_code_before_optimization;
  }

  if (!target_code_before_regalloc.empty()) {
    ret << title("Target code before regAlloc()")
        << target_code_before_regalloc;
  }

  if (!target_code_before_liveness.empty()) {
    ret << title("Target code before liveness, after peepholes")
        << target_code_before_liveness;
  }

  return ret;
}


void CompileData::clear() {
  liveness_dump.clear();
  target_code_before_optimization.clear();
  target_code_before_regalloc.clear();
  target_code_before_liveness.clear();
  allocated_registers_dump.clear();
  num_accs_introduced = 0;
}

}  // namespace V3DLib
