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

}  // namespace V3DLib
