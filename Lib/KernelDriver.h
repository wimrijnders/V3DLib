#ifndef _LIB_KERNELDRIVER_H
#define _LIB_KERNELDRIVER_H
#include <vector>
#include <string>
#include "Common/BufferType.h"
#include "Common/CompileData.h"
#include "Source/StmtStack.h"
#include "Target/CFG.h"

namespace V3DLib {

class KernelDriver {
public:
  KernelDriver(BufferType in_buffer_type) : buffer_type(in_buffer_type) {}
  KernelDriver(KernelDriver &&k) = default;
  virtual ~KernelDriver();

  BufferType const buffer_type;

  virtual void encode() = 0;

  void compile();
  void invoke(int numQPUs, IntList &params);
  void pretty(int numQPUs, const char *filename = nullptr, bool output_qpu_code = true);
  int numVars() const { return m_numVars; }
  bool has_errors() const { return !errors.empty(); }
  std::string get_errors() const;
  void dump_compile_data(char const *filename) const;
  std::string compile_info() const;

  Stmt::Ptr sourceCode() { return m_body; }  //<< return AST representing the source code
  Instr::List &targetCode() { return m_targetCode; }

protected:
  const int MAX_KERNEL_PARAMS = 128;  // Maximum number of kernel parameters allowed

  Instr::List m_targetCode;           // Target code generated from AST
  Stmt::Ptr   m_body;

  int qpuCodeMemOffset = 0;
  std::vector<std::string> errors;

  void init_compile(bool set_qpu_uniforms = true, int numVars = 0);
  virtual void emit_opcodes(FILE *f) {} 
  void obtain_ast();
  void add_stmt(Stmt::Ptr stmt);

private:
  StmtStack m_stmtStack;
  int m_numVars = 0;                  // The number of variables in the source code for vc4
  CompileData m_compile_data;

  virtual void compile_intern() = 0;
  virtual void invoke_intern(int numQPUs, IntList &params) = 0;

  int numAccs() const { return m_compile_data.num_accs_introduced; }

  bool handle_errors();
};


void compile_postprocess(Instr::List &targetCode);

}  // namespace V3DLib

#endif  // _LIB_vc4_KERNELDRIVER_H
