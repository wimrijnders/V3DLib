#ifndef _LIB_KERNELDRIVER_H
#define _LIB_KERNELDRIVER_H
#include <stdio.h>
#include <vector>
#include <string>
#include "Common/Stack.h"
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
	virtual void invoke(int numQPUs, Seq<int32_t>* params) = 0;
	virtual void encode(int numQPUs) = 0;

	void pretty(const char *filename = nullptr);
	void init_compile();
	void compile();

	/**
	 * @return AST representing the source code
	 */
	Stmt *sourceCode() { return body; }

	Seq<Instr> &targetCode() { return m_targetCode; }

	bool handle_errors();

	BufferType const buffer_type;


protected:
	const int MAX_KERNEL_PARAMS = 128;  // Maximum number of kernel parameters allowed

  Seq<Instr> m_targetCode;            // Target code generated from AST

  int qpuCodeMemOffset = 0;
	std::vector<std::string> errors;

	virtual void emit_opcodes(FILE *f) {} 

private:
	Stack<Stmt> m_stmtStack;
  Stmt *body = nullptr;

	void print_source_code(FILE *f);
	void emit_target_code(FILE *f);
};


#ifdef DEBUG

// Expose for unit tests
void compileKernel(Seq<Instr>* targetCode, Stmt* body);

#endif

}  // namespace QPULib

#endif  // _LIB_vc4_KERNELDRIVER_H
