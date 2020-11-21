#ifndef _LIB_VC4_KERNELDRIVER_H
#define _LIB_VC4_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "Source/Stmt.h"
#include "Invoke.h"

namespace V3DLib {
namespace vc4 {

class KernelDriver : public V3DLib::KernelDriver {
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
}  // namespace V3DLib


#endif  // _LIB_VC4_KERNELDRIVER_H
