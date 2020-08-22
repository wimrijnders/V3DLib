#ifndef _LIB_VC4_KERNELDRIVER_H
#define _LIB_VC4_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "Source/Stmt.h"
#include "Target/Encode.h"
#include "Invoke.h"


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
  SharedArray<uint32_t> qpuCodeMem;
};

}  // namespace vc4
}  // namespace QPULib


#endif  // _LIB_VC4_KERNELDRIVER_H
