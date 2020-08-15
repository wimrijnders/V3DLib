#ifndef _LIB_V3D_KERNELDRIVER_H
#define _LIB_V3D_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "SharedArray.h"

#ifdef QPU_MODE

namespace QPULib {
namespace v3d {

class KernelDriver : public QPULib::KernelDriver {
public:
	KernelDriver();
	~KernelDriver() override;

	void encode(Seq<Instr> &targetCode) override;
	void invoke(int numQPUs, Seq<int32_t>* params) override;

	BufferType const buffer_type = BufferType::V3dBuffer;

private:
  SharedArray<uint64_t>* qpuCodeMem = nullptr;
  SharedArray<uint32_t>* paramMem   = nullptr;
};

}  // namespace v3d
}  // namespace QPULib

#endif  // QPU_MODE

#endif  // _LIB_V3d_KERNELDRIVER_H
