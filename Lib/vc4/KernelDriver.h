#ifndef _LIB_VC4_KERNELDRIVER_H
#define _LIB_VC4_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "SharedArray.h"
#include "Source/Stmt.h"
#include "Target/Encode.h"
#include "Invoke.h"

#ifdef QPU_MODE

namespace QPULib {
namespace vc4 {

class KernelDriver : public QPULib::KernelDriver {
public:
	KernelDriver();
	~KernelDriver() override;

	void kernelFinish() override;
	void encode(int numQPUs, Seq<Instr> &targetCode) override;
	void invoke(int numQPUs, Seq<int32_t>* params) override;

private:
  // Memory region for QPU code and parameters
  vc4::SharedArray<uint32_t>* qpuCodeMem = nullptr;
};

}  // namespace vc4
}  // namespace QPULib

#endif  // QPU_MODE

#endif  // _LIB_VC4_KERNELDRIVER_H
