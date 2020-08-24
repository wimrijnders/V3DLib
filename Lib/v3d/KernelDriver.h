#ifndef _LIB_V3D_KERNELDRIVER_H
#define _LIB_V3D_KERNELDRIVER_H
#include "../KernelDriver.h"
#include "Common/SharedArray.h"
#include "instr/Instr.h"

#ifdef QPU_MODE

namespace QPULib {
namespace v3d {

class KernelDriver : public QPULib::KernelDriver {
	using Instruction  = QPULib::v3d::instr::Instr;
	using Instructions = std::vector<Instruction>;

public:
	KernelDriver();

	void encode(int numQPUs) override;
	void invoke(int numQPUs, Seq<int32_t>* params) override;

private:
  SharedArray<uint64_t> qpuCodeMem;
  SharedArray<uint32_t> paramMem;
	Instructions          instructions;

	void emit_opcodes(FILE *f) override;
};

}  // namespace v3d
}  // namespace QPULib

#endif  // QPU_MODE

#endif  // _LIB_V3d_KERNELDRIVER_H
