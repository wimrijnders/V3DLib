#ifndef _LIB_KERNELDRIVER_H
#define _LIB_KERNELDRIVER_H
#include <stdio.h>
#include <vector>
#include <string>
#include "Target/CFG.h"
#include "Common/SharedArray.h"

namespace QPULib {

class Stmt;

class KernelDriver {
public:
	KernelDriver(BufferType in_buffer_type) : buffer_type(in_buffer_type) {}
	virtual ~KernelDriver() {
		// TODO: shouldn't body be cleaned up?
	}

	virtual void kernelFinish() {} 
	virtual void encode(int numQPUs, Seq<QPULib::Instr> &targetCode) = 0;
	virtual void invoke(int numQPUs, Seq<int32_t>* params) = 0;
  virtual void pretty(FILE *f) { print_source_code(f); }

	void transfer_stack();

	/**
	 * @return AST representing the source code
	 */
	Stmt *sourceCode() { return body; }

	bool handle_errors();

	BufferType const buffer_type;

protected:
	const int MAX_KERNEL_PARAMS = 128;  // Maximum number of kernel parameters allowed

  Stmt *body = nullptr;
  int qpuCodeMemOffset = 0;
	std::vector<std::string> errors;

	void print_source_code(FILE *f);
};

}  // namespace QPULib

#endif  // _LIB_vc4_KERNELDRIVER_H
