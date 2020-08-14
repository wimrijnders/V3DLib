#ifndef _LIB_KERNELDRIVERS_h
#define _LIB_KERNELDRIVERS_h
#include "Target/CFG.h"
#include "Common/SharedArray.h"
//#include "VideoCore/SharedArray.h"
//#include "v3d/SharedArray.h"
//#include "v3d/BufferObject.h"  // ArrayView<>


namespace QPULib {

class KernelDriver {
public:
	KernelDriver(BufferType in_buffer_type) : buffer_type(in_buffer_type) {}
	virtual ~KernelDriver() {}

	virtual void kernelFinish() {} 
	virtual void encode(Seq<Instr> &targetCode) = 0;
	virtual void invoke(int numQPUs, Seq<int32_t>* params) = 0;
  virtual void pretty(FILE *f) { /* nothing to do yet */ }

	BufferType const buffer_type;

protected:
	// Maximum number of kernel parameters allowed
	const int MAX_KERNEL_PARAMS = 128;

  int qpuCodeMemOffset = 0;
};


namespace vc4 {

class KernelDriver : public QPULib::KernelDriver {
public:
	KernelDriver();
	~KernelDriver() override;

	void kernelFinish() override;
	void encode(Seq<Instr> &targetCode) override;
	void invoke(int numQPUs, Seq<int32_t>* params) override;

	BufferType const buffer_type = BufferType::Vc4Buffer;

private:
  // Memory region for QPU code and parameters
  VideoCore::SharedArray<uint32_t>* qpuCodeMem = nullptr;
};

}  // namespace vc4


namespace v3d {

class KernelDriver : public QPULib::KernelDriver {
public:
	KernelDriver();
	~KernelDriver() override;

	void encode(Seq<Instr> &targetCode) override;
	void invoke(int numQPUs, Seq<int32_t>* params) override;

	BufferType const buffer_type = BufferType::V3dBuffer;

private:
  v3d::SharedArray<uint64_t>* qpuCodeMem = nullptr;
  v3d::SharedArray<uint32_t>* paramMem   = nullptr;
};

}  // namespace v3d
}  // namespace QPULib

#endif  // _LIB_KERNELDRIVERS_h
