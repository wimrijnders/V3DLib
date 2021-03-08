#ifndef _V3DLIB_COMMON_COMPILEDATA_H_
#define _V3DLIB_COMMON_COMPILEDATA_H_
#include <string>
#include <vector>
#include "Target/instr/Reg.h"

namespace V3DLib {

struct CompileData {
  std::string liveness_dump;
  std::string target_code_before_regalloc;
  std::string allocated_registers_dump;
  int num_accs_introduced = 0;

  void allocated_registers(std::vector<Reg> const &allocated_regs);
  void clear();
};

extern CompileData compile_data;

}  // namespace V3DLib

#endif  // _V3DLIB_COMMON_COMPILEDATA_H_
