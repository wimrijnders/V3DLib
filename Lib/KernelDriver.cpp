#include "KernelDriver.h"
#include <iostream>            // cout
#include "Support/basics.h"
#include "Source/StmtStack.h"
#include "Source/Pretty.h"
#include "Source/Translate.h"
#include "Source/Lang.h"       // initStmt
#include "Target/Satisfy.h"
#include "Target/RemoveLabels.h"
#include "SourceTranslate.h"


namespace V3DLib {

using ::operator<<;  // C++ weirdness

namespace {

void print_source_code(FILE *f, Stmt *body) {
	// Emit source code
	fprintf(f, "Source code\n");
	fprintf(f, "===========\n\n");
	if (body == nullptr)
		fprintf(stderr, "<No source code to print>");
	else
		V3DLib::pretty(f, body);

	fprintf(f, "\n");
	fflush(f);
}


void print_target_code(FILE *f, Seq<Instr> const &code) {
	fprintf(f, "Target code\n");
	fprintf(f, "===========\n\n");
	fprintf(f, mnemonics(code, true).c_str());
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
void compile_postprocess(Seq<Instr> &targetCode) {
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

  // Translate branch-to-labels to relative branches
	if (Platform::instance().compiling_for_vc4()) {  // For v3d, it happens in `v3d::KernelDriver::to_opcodes()` 
	  removeLabels(targetCode);
	}
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
	initStmt(m_stmtStack);
	resetFreshVarGen(numVars);
	resetFreshLabelGen();

	if (set_qpu_uniforms) {
		// Reserved general-purpose variables
		Int qpuId, qpuCount;
		qpuId    = getUniformInt();
		qpuCount = getUniformInt();
	}
}


void KernelDriver::obtain_ast() {
	finishStmt();
	m_body = m_stmtStack.pop();
}


void KernelDriver::_compile() {
	kernelFinish();
	obtain_ast();
 	translateStmt(m_targetCode, m_body);

	if (Platform::instance().compiling_for_vc4()) {
	  m_targetCode << Instr(END);
	}

	getSourceTranslate().add_init(m_targetCode);  // TODO init block only added for v3d, refactor

	compile_postprocess(m_targetCode);
}


void KernelDriver::compile() {
	try {
		_compile();
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


#ifdef DEBUG
// Only here for autotest
void KernelDriver::add_stmt(Stmt *stmt) {
	m_stmtStack.append(stmt);
}
#endif

}  // namespace V3DLib
