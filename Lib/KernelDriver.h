#ifndef _LIB_KERNELDRIVER_H
#define _LIB_KERNELDRIVER_H
#include <stdio.h>
#include <vector>
#include <string>
#include "Target/CFG.h"
#include "Common/SharedArray.h"

namespace QPULib {

class KernelDriver {
public:
	KernelDriver(BufferType in_buffer_type) : buffer_type(in_buffer_type) {}
	virtual ~KernelDriver() {}

	virtual void kernelFinish() {} 
	virtual void encode(int numQPUs, Seq<QPULib::Instr> &targetCode) = 0;
	virtual void invoke(int numQPUs, Seq<int32_t>* params) = 0;
  virtual void pretty(FILE *f) { /* nothing to do yet */ }

	bool handle_errors();

	BufferType const buffer_type;

protected:
	// Maximum number of kernel parameters allowed
	const int MAX_KERNEL_PARAMS = 128;

  int qpuCodeMemOffset = 0;
	std::vector<std::string> errors;
};

}  // namespace QPULib

#endif  // _LIB_vc4_KERNELDRIVER_H
