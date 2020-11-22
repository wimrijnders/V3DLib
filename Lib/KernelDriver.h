#ifndef _LIB_KERNELDRIVER_H
#define _LIB_KERNELDRIVER_H
#include <stdio.h>
#include <vector>
#include <string>
#include "Common/Stack.h"
#include "Target/CFG.h"
#include "Common/SharedArray.h"

namespace V3DLib {

class Stmt;

class KernelDriver {
public:
	KernelDriver(BufferType in_buffer_type) : buffer_type(in_buffer_type) {}
	virtual ~KernelDriver();

	virtual void kernelFinish() {} 
	virtual void encode(int numQPUs) = 0;

	void init_compile();
	void compile();
	void invoke(int numQPUs, Seq<int32_t> &params);
	void pretty(int numQPUs, const char *filename = nullptr);

	/**
	 * @return AST representing the source code
	 */
	Stmt *sourceCode() { return m_body; }

	Seq<Instr> &targetCode() { return m_targetCode; }

	BufferType const buffer_type;

protected:
	const int MAX_KERNEL_PARAMS = 128;  // Maximum number of kernel parameters allowed

  Seq<Instr> m_targetCode;            // Target code generated from AST

  int qpuCodeMemOffset = 0;
	std::vector<std::string> errors;

	virtual void emit_opcodes(FILE *f) {} 
	void obtain_ast();

private:
	Stack<Stmt> m_stmtStack;
  Stmt       *m_body = nullptr;

	void _compile();
	virtual void invoke_intern(int numQPUs, Seq<int32_t>* params) = 0;
	bool has_errors() const { return !errors.empty(); }
	bool handle_errors();
};


// Exposed for unit test `testAutoTest`
void compileKernel(Seq<Instr> &targetCode, Stmt* body);

}  // namespace V3DLib

#endif  // _LIB_vc4_KERNELDRIVER_H
