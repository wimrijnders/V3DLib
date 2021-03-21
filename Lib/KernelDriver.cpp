#include "KernelDriver.h"
#include <iostream>            // cout
#include "Support/basics.h"
#include "Support/Platform.h"
#include "Source/StmtStack.h"
#include "Source/Pretty.h"
#include "Source/Translate.h"
#include "Source/Lang.h"       // initStmt
#include "Target/Satisfy.h"
#include "SourceTranslate.h"
#include "Support/Timer.h"

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace {

FILE *open_file(char const *filename, char const *label) {
  assert(label != nullptr);

  FILE *f = nullptr;

  if (filename == nullptr)
    f = stdout;
  else {
    f = fopen(filename, "w");
    if (f == nullptr) {
      fprintf(stderr, "ERROR: could not open file '%s' for %s output\n", filename, label);
    }
  }

  return f;
}


void title(FILE *f, std::string const &title) {
  assert(f != nullptr);
  assert(!title.empty());

  fprintf(f, "\n");
  fprintf(f, title.c_str());
  fprintf(f, "\n");

  std::string line(title.size(), '=');
  fprintf(f, line.c_str());
  fprintf(f, "\n");
}


/**
 * Emit source code
 */
void print_source_code(FILE *f, Stmt::Ptr body) {
  if (f == nullptr) {
    f = stdout;
  }

  title(f, "Source code");

  if (body.get() == nullptr) {
    fprintf(f, "<No source code to print>\n");
  } else {
    fprintf(f, pretty(body).c_str());
  }

  fprintf(f, "\n");
  fflush(f);
}


void print_target_code(FILE *f, Instr::List const &code) {
  title(f, "Target code");

  if (code.empty()) {
    fprintf(f, "<No target code to print>\n");
  } else {
    fprintf(f, code.mnemonics(true).c_str());
  }
  fprintf(f, "\n");
  fflush(f);
}

}  // anon namespace


///////////////////////////////////////////////////////////////////////////////
// Compile kernel
///////////////////////////////////////////////////////////////////////////////

/**
 * @param targetCode  output variable for the target code assembled from the AST and adjusted
 */
void compile_postprocess(Instr::List &targetCode) {
  assertq(!targetCode.empty(), "compile_postprocess(): passed target code is empty");

  loadStorePass(targetCode);

  // Construct control-flow graph
  CFG cfg;
  cfg.build(targetCode);

  //compile_data.target_code_before_regalloc = targetCode.dump();

  // Perform register allocation
  getSourceTranslate().regAlloc(&cfg, targetCode);

  // Satisfy target code constraints
  satisfy(targetCode);
}


/**
 * NOTE: Don't clean up `body` here, it's a pointer to the top of the AST.
 */
KernelDriver::~KernelDriver() {}


/**
 * Reset the state for compilation
 *
 * The parameters are only here for autotest unit test.
 *
 * @param set_qpu_uniforms  if true, initialize the uniforms for QPU ID and number of QPU's
 * @param numVars           number of variables already assigned prior to compilation
 */
void KernelDriver::init_compile() {
  initStmt();
  initStack(m_stmtStack);
  resetFreshVarGen();
  resetFreshLabelGen();
  Pointer::reset_increment();
  compile_data.clear();

  // Initialize reserved general-purpose variables
  Int qpuId, qpuCount;
  qpuId    = getUniformInt();
  qpuCount = getUniformInt();
}


void KernelDriver::obtain_ast() {
  clearStack();
  m_body = m_stmtStack.pop();
}


/**
 * Entry point for compilation of source code to target code.
 *
 * This method is here to just handle thrown exceptions.
 */
void KernelDriver::compile(std::function<void()> create_ast) {
  try {
    create_ast();
    compile_intern();
    m_numVars = getFreshVarCount();
    m_compile_data = compile_data;
  } catch (V3DLib::Exception const &e) {
    std::string msg = "Exception occured during compilation: ";
    msg << e.msg();

    if (e.msg().compare(0, 5, "ERROR") == 0) {
      errors << msg;
    } else {
      throw;  // Must be a fatal()
    }

    clearStack();
  }
}


std::string KernelDriver::get_errors() const {
  std::string ret;

  for (auto const &err : errors) {
    ret << "  " << err << "\n";
  }

  return ret;
}


/**
 * @return true if errors present, false otherwise
 */
bool KernelDriver::handle_errors() {
  using std::cout;
  using std::endl;

  if (errors.empty()) return false;

  cout << "Errors encountered during compilation and/or encoding:\n"
       << get_errors()
       << "\nNot running the kernel" << endl;

  return true;      
}


/**
* @brief Output a human-readable representation of the source and target code.
*
* @param filename  if specified, print the output to this file. Otherwise, print to stdout
*/
void KernelDriver::pretty(char const *filename, bool output_qpu_code) {
  FILE *f = open_file(filename, "pretty");
  if (f == nullptr) return;

  if (has_errors()) {
    fprintf(f, "=== There were errors during compilation, the output here is likely incorrect or incomplete  ===\n");
    fprintf(f, "=== Encoding and displaying output as best as possible                                       ===\n");
    fprintf(f, "\n\n");
  }

  print_source_code(f, sourceCode());
  print_target_code(f, m_targetCode);

  if (output_qpu_code) {
    emit_opcodes(f);
  }

  if (filename != nullptr) {
    assert(f != nullptr);
    assert(f != stdout);
    fclose(f);
  }
}


void KernelDriver::dump_compile_data(char const *filename) const {
  FILE *f = open_file(filename, "compile_data");
  if (f == nullptr) return;

  title(f, "Liveness dump");
  fprintf(f, m_compile_data.liveness_dump.c_str());

  title(f, "Allocated registers to variables");
  fprintf(f, m_compile_data.allocated_registers_dump.c_str());

  if (!m_compile_data.target_code_before_regalloc.empty()) {
    title(f, "Target code before regAlloc()");
    fprintf(f, m_compile_data.target_code_before_regalloc.c_str());
  }

  if (!m_compile_data.target_code_before_liveness.empty()) {
    title(f, "Target code before liveness, after peepholes");
    fprintf(f, m_compile_data.target_code_before_liveness.c_str());
  }

  title(f, "ACC usage");
  fprintf(f, m_targetCode.check_acc_usage().c_str());

  if (filename != nullptr) {
    fclose(f);
  }
}


void KernelDriver::invoke(int numQPUs, IntList &params) {
  assert(params.size() != 0);

  if (handle_errors()) {
    fatal("Errors during kernel compilation/encoding, can't continue.");
  }

   // Invoke kernel on QPUs
  invoke_intern(numQPUs, params);
}


std::string KernelDriver::compile_info() const {
  std::string ret;

  ret << "  compile num generated variables: " << numVars() << "\n"
      << "  num accs introduced            : " << numAccs() << "\n"
      << "  num compile errors             : " << errors.size();

  return ret;
}

}  // namespace V3DLib
