#ifndef _LIB_KERNELDRIVER_H
#define _LIB_KERNELDRIVER_H
#include <vector>
#include <string>
#include "Common/BufferType.h"
#include "Source/StmtStack.h"
#include "Target/CFG.h"

namespace V3DLib {

class KernelDriver {
public:
  KernelDriver(BufferType in_buffer_type) : buffer_type(in_buffer_type) {}
  virtual ~KernelDriver();

  virtual void encode(int numQPUs) = 0;

  void compile();
  void invoke(int numQPUs, Seq<int32_t> &params);
  void pretty(int numQPUs, const char *filename = nullptr);

  /**
   * @return AST representing the source code
   */
  Stmt::Ptr sourceCode() { return m_body; }

  Seq<Instr> &targetCode() { return m_targetCode; }

  BufferType const buffer_type;

  void add_stmt(Stmt::Ptr stmt);  // exposed for autotest


protected:
  const int MAX_KERNEL_PARAMS = 128;  // Maximum number of kernel parameters allowed

  Instr::List m_targetCode;           // Target code generated from AST
  Stmt::Ptr   m_body;

  int qpuCodeMemOffset = 0;
  std::vector<std::string> errors;

  void init_compile(bool set_qpu_uniforms = true, int numVars = 0);
  virtual void emit_opcodes(FILE *f) {} 
  void obtain_ast();


private:
  StmtStack m_stmtStack;

  virtual void compile_intern() = 0;
  virtual void invoke_intern(int numQPUs, Seq<int32_t>* params) = 0;

  bool has_errors() const { return !errors.empty(); }
  bool handle_errors();
};

void compile_postprocess(Instr::List &targetCode);

}  // namespace V3DLib

#endif  // _LIB_vc4_KERNELDRIVER_H
