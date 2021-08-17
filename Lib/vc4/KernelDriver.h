#ifndef _LIB_VC4_KERNELDRIVER_H
#define _LIB_VC4_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "Invoke.h"

namespace V3DLib {
namespace vc4 {

class KernelDriver : public V3DLib::KernelDriver {
  using Parent = V3DLib::KernelDriver;

public:
  KernelDriver();
  KernelDriver(KernelDriver &&k) = default;

  void encode() override;
  int kernel_size() const { return (int) code.size()/2; }

private:
  Code qpuCodeMem;     // Memory region for QPU code and parameters
                       // Doesn't survive std::move, dtor gets called despite move ctor present
  Data uniforms;

  /**
   * Container for launch info per QPU to run
   *
   * Array consecutively containing two values per QPU to run:
   *  - pointer to uniform parameters to pass per QPU
   *  - Start of code block to run per QPU
   *
   * The uniforms are essentially the same for all QPUs, *except* qpu id, the first parameter.
   *
   * It should thus be possible to run different code per QPU.
   * Haven't tried this yet, till now all the QPUs run the same code.
   */
  Data launch_messages;

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
