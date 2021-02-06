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

namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace {

/**
 * Emit source code
 */
void print_source_code(FILE *f, Stmt::Ptr body) {
  if (f == nullptr) {
    f = stdout;
  }

  fprintf(f, "Source code\n");
  fprintf(f, "===========\n\n");

  if (body.get() == nullptr) {
    fprintf(f, "<No source code to print>\n");
  } else {
    fprintf(f, pretty(body).c_str());
  }

  fprintf(f, "\n");
  fflush(f);
}


void print_target_code(FILE *f, Instr::List const &code) {
  fprintf(f, "Target code\n");
  fprintf(f, "===========\n\n");
  if (code.empty()) {
    fprintf(f, "<No target code to print>\n");
  } else {
    fprintf(f, code.mnemonics(true).c_str());
  }
  fprintf(f, "\n");
  fflush(f);
}

}  // anon namespace

// ============================================================================
// Compile kernel
// ============================================================================

/**
 * @param targetCode  output variable for the target code assembled from the AST and adjusted
 */
void compile_postprocess(Instr::List &targetCode) {
  assertq(!targetCode.empty(), "compile_postprocess(): passed target code is empty");

  // Load/store pass
  loadStorePass(targetCode);

  // Construct control-flow graph
  CFG cfg;
  buildCFG(targetCode, cfg);

  // Perform register allocation
  getSourceTranslate().regAlloc(&cfg, &targetCode);

  // Satisfy target code constraints
  satisfy(&targetCode);
}


/**
 * Don't clean up `body` here, it's a pointer to the top of the AST.
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
void KernelDriver::init_compile(bool set_qpu_uniforms, int numVars) {
  initStmt();
  initStack(m_stmtStack);
  resetFreshVarGen(numVars);
  resetFreshLabelGen();
  Pointer::reset_increment();

  if (set_qpu_uniforms) {
    // Reserved general-purpose variables
    Int qpuId, qpuCount;
    qpuId    = getUniformInt();
    qpuCount = getUniformInt();
  }
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
void KernelDriver::compile() {
  try {
    compile_intern();
  } catch (V3DLib::Exception const &e) {
    std::string msg = "Exception occured during compilation: ";
    msg << e.msg();

    if (e.msg().compare(0, 5, "ERROR") == 0) {
      errors << msg;
    } else {
      throw;  // Must be a fatal()
    }
  }
}


/**
 * @return true if errors present, false otherwise
 */
bool KernelDriver::handle_errors() {
  using std::cout;
  using std::endl;

  if (errors.empty()) {
    return false;
  }

  cout << "Errors encountered during compilation and/or encoding:\n";

  for (auto const &err : errors) {
    cout << "  " << err << "\n";
  }

  cout << "\nNot running the kernel" << endl;
  return true;      
}


/**
* @brief Output a human-readable representation of the source and target code.
*
* @param filename  if specified, print the output to this file. Otherwise, print to stdout
*/
void KernelDriver::pretty(int numQPUs, const char *filename) {
  FILE *f = nullptr;

  if (filename == nullptr)
    f = stdout;
  else {
    f = fopen(filename, "w");
    if (f == nullptr) {
      fprintf(stderr, "ERROR: could not open file '%s' for pretty output\n", filename);
      return;
    }
  }


  if (has_errors()) {
    fprintf(f, "=== There were errors during compilation, the output here is likely incorrect or incomplete  ===\n");
    fprintf(f, "=== Encoding and displaying output as best as possible                                       ===\n");
    fprintf(f, "\n\n");
  }

  print_source_code(f, sourceCode());
  print_target_code(f, m_targetCode);

  if (!has_errors()) {
    encode(numQPUs);  // generate opcodes if not already done
  }
  emit_opcodes(f);

  if (filename != nullptr) {
    assert(f != nullptr);
    assert(f != stdout);
    fclose(f);
  }
}


void KernelDriver::invoke(int numQPUs, Seq<int32_t> &params) {
  if (!has_errors()) {
    encode(numQPUs);
  }

  if (handle_errors()) {
    fatal("Errors during kernel compilation/encoding, can't continue.");
  }

   // Invoke kernel on QPUs
  invoke_intern(numQPUs, &params);
}


/**
 * Only here for autotest
 */
void KernelDriver::add_stmt(Stmt::Ptr stmt) {
  m_stmtStack << stmt;
}

}  // namespace V3DLib
