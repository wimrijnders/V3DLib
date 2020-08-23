#ifndef _LIB_V3D_KERNELDRIVER_H
#define _LIB_V3D_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"

#ifdef QPU_MODE

namespace QPULib {
namespace v3d {

class KernelDriver : public QPULib::KernelDriver {
public:
	KernelDriver();

	void encode(int numQPUs) override;
	void invoke(int numQPUs, Seq<int32_t>* params) override;

private:
  SharedArray<uint64_t> qpuCodeMem;
  SharedArray<uint32_t> paramMem;
	std::vector<uint64_t> code;                   // opcodes for v3d

	void emit_opcodes(FILE *f) override;
};

}  // namespace v3d
}  // namespace QPULib

#endif  // QPU_MODE

#endif  // _LIB_V3d_KERNELDRIVER_H
