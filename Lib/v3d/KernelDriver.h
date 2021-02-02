#ifndef _LIB_V3D_KERNELDRIVER_H
#define _LIB_V3D_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "instr/Instr.h"

#ifdef QPU_MODE

namespace V3DLib {
namespace v3d {

class KernelDriver : public V3DLib::KernelDriver {
  using Parent       = V3DLib::KernelDriver;
  using Instruction  = V3DLib::v3d::instr::Instr;
  using Instructions = V3DLib::v3d::Instructions;

public:
  KernelDriver();

  void compile_init();
  void encode(int numQPUs) override;

private:
  SharedArray<uint64_t> qpuCodeMem;
  SharedArray<uint32_t> paramMem;
  Instructions          instructions;

  void compile_intern() override;
  void invoke_intern(int numQPUs, Seq<int32_t>* params) override;

  std::vector<uint64_t> to_opcodes();
  void emit_opcodes(FILE *f) override;
};

}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE

#endif  // _LIB_V3d_KERNELDRIVER_H
