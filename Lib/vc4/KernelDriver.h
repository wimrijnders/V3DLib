#ifndef _LIB_VC4_KERNELDRIVER_H
#define _LIB_VC4_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "Invoke.h"

namespace V3DLib {
namespace vc4 {

class KernelDriver : public V3DLib::KernelDriver, private MailBoxInvoke {
  using Parent = V3DLib::KernelDriver;

public:
  KernelDriver();
  KernelDriver(KernelDriver &&k) = default;

  void encode() override;
  int kernel_size() const;

private:
  Code qpuCodeMem;     // Memory region for QPU code
                       // Doesn't survive std::move, dtor gets called despite move ctor present

  void kernelFinish();
  void compile_intern() override;
  void invoke_intern(int numQPUs, IntList &params) override;

  void emit_opcodes(FILE *f) override;
};

}  // namespace vc4
}  // namespace V3DLib

#endif  // _LIB_VC4_KERNELDRIVER_H
