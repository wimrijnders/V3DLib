#ifndef _LIB_V3D_KERNELDRIVER_H
#define _LIB_V3D_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "instr/Instr.h"
#include "BufferObject.h"

namespace V3DLib {
namespace v3d {

/**
 * Buffer Object for v3d
 *
 * The generated code is loaded into a separate BO.
 * This is to be able to load and run multiple kernels for v3d in the same context.
 *
 * Loading multiple kernels into the same Buffer Object (BO) didn't work for unfathomable reasons,
 * and resulted in run timeouts and eventually locked up the pi4.
 * vc4 does not have this issue.
 */
class KernelDriver : public V3DLib::KernelDriver {
  using Parent       = V3DLib::KernelDriver;
  using Instruction  = V3DLib::v3d::instr::Instr;
  using Instructions = V3DLib::v3d::Instructions;
  using Code         = SharedArray<uint64_t>;
  using Params       = SharedArray<uint32_t>;

public:
  KernelDriver();
  KernelDriver(KernelDriver &&a) = default;

  void encode() override;
  int kernel_size() const { return (int) instructions.size(); }

private:
  Params        paramMem;
  Instructions  instructions;
  BufferObject  code_bo;
  Code          qpuCodeMem;
  SharedArray<uint32_t> devnull;

  void compile_intern() override;
  void invoke_intern(int numQPUs, IntList &params) override;

  void allocate();
  std::vector<uint64_t> to_opcodes();
  void emit_opcodes(FILE *f) override;
};

}  // namespace v3d
}  // namespace V3DLib

#endif  // _LIB_V3d_KERNELDRIVER_H
