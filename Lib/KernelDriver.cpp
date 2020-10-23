#include "KernelDriver.h"
#include <iostream>            // cout
#include "Support/basics.h"
#include "Source/Syntax.h"     // stmtStack
#include "Source/Pretty.h"
#include "Source/Translate.h"
#include "Source/Stmt.h"       // initStmt
#include "Target/Satisfy.h"
#include "Target/RemoveLabels.h"
#include "SourceTranslate.h"


namespace QPULib {

// ============================================================================
// Compile kernel
// ============================================================================

/**
 * @param body        top of the AST
 * @param targetCode  output variable for the target code assembled from the AST and adjusted
 */
void compileKernel(Seq<Instr> &targetCode, Stmt* body) {
	assert(body != nullptr);

  // Translate to target code
  translateStmt(&targetCode, body);

	getSourceTranslate().add_init(targetCode);

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


void KernelDriver::init_compile() {
	initStmt(m_stmtStack);
	resetFreshVarGen();
	resetFreshLabelGen();

	// Reserved general-purpose variables
	Int qpuId, qpuCount;
	qpuId = getUniformInt();
	qpuCount = getUniformInt();
}


void KernelDriver::compile() {
	kernelFinish();
	finishStmt();

	// Obtain the AST
	body = m_stmtStack.top();
	m_stmtStack.pop();

	try {
		compileKernel(m_targetCode, body);
	} catch (QPULib::Exception const &e) {
		std::string msg = "Exception occured during compilation: ";
		msg += e.msg();  // WHY doesn't << work here???
		//std::cerr << msg << std::endl;

		if (e.msg().compare(0, 5, "ERROR") == 0) {
			errors.push_back(msg); // WHY doesn't << work here???
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
		fprintf(f, "=== Encoding and displaying output as best as possible                                       ===\n\n\n");
	}

	print_source_code(f);
	emit_target_code(f);

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


void KernelDriver::print_source_code(FILE *f) {
	// Emit source code
	fprintf(f, "Source code\n");
	fprintf(f, "===========\n\n");
	if (body == nullptr)
		fprintf(stderr, "<No source code to print>");
	else
		QPULib::pretty(f, body);

	fprintf(f, "\n");
	fflush(f);
}


void KernelDriver::emit_target_code(FILE *f) {
	// Emit target code
	fprintf(f, "Target code\n");
	fprintf(f, "===========\n\n");

	for (int i = 0; i < m_targetCode.size(); i++) {
		auto &instr = m_targetCode[i];
		fprintf(f, "%i: %s", i, instr.mnemonic(true).c_str());
	}

	fprintf(f, "\n");
	fflush(f);
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

}  // namespace QPULib
