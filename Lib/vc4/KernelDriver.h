#ifndef _LIB_VC4_KERNELDRIVER_H
#define _LIB_VC4_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "Invoke.h"

namespace V3DLib {
namespace vc4 {

class KernelDriver : public V3DLib::KernelDriver {
  using Code   = SharedArray<uint32_t>;
  using Parent = V3DLib::KernelDriver;

public:
  KernelDriver();
  KernelDriver(KernelDriver &&k) = default;

  void compile_init(bool set_qpu_uniforms = true, int numVars = 0);
  void encode() override;

private:
  Code qpuCodeMem;     // Memory region for QPU code and parameters
  UIntList code;       // opcodes for vc4; can't convert this to uint64_t because qpuCodeMem
                       // needs to be uint32_t for invoking (tried it, don't try again!).
  void kernelFinish();
  void compile_intern() override;
  void invoke_intern(int numQPUs, IntList &params) override;

  void emit_opcodes(FILE *f) override;
};

}  // namespace vc4
}  // namespace V3DLib


#endif  // _LIB_VC4_KERNELDRIVER_H
