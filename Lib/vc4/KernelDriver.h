#ifndef _LIB_VC4_KERNELDRIVER_H
#define _LIB_VC4_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "Source/Stmt.h"
#include "Invoke.h"

namespace QPULib {
namespace vc4 {

class KernelDriver : public QPULib::KernelDriver {
public:
	KernelDriver();

	void kernelFinish() override;
	void encode(int numQPUs) override;

protected:
	void emit_opcodes(FILE *f) override;

private:
  SharedArray<uint32_t> qpuCodeMem;   // Memory region for QPU code and parameters
  Seq<uint32_t> code;                 // opcodes for vc4

	void invoke_intern(int numQPUs, Seq<int32_t>* params) override;
};

}  // namespace vc4
}  // namespace QPULib


#endif  // _LIB_VC4_KERNELDRIVER_H
