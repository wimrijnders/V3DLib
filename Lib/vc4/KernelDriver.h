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
	void invoke(int numQPUs, Seq<int32_t>* params) override;
	void encode(int numQPUs) override;

private:
  SharedArray<uint32_t> qpuCodeMem;   // Memory region for QPU code and parameters
  Seq<uint32_t> code;                 // opcodes for vc4
};

}  // namespace vc4
}  // namespace QPULib


#endif  // _LIB_VC4_KERNELDRIVER_H
