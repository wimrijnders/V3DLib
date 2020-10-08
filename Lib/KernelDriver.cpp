#include "KernelDriver.h"
#include <iostream>            // cout
#include "Support/debug.h"
#include "Source/Syntax.h"     // stmtStack
#include "Source/Pretty.h"
#include "Source/Translate.h"
#include "Source/Stmt.h"       // initStmt
#include "Target/Pretty.h"
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
void compileKernel(Seq<Instr>* targetCode, Stmt* body) {
	assert(targetCode != nullptr);
	assert(body != nullptr);

  // Translate to target code
  translateStmt(targetCode, body);

	getSourceTranslate().add_init(*targetCode);

  // Load/store pass
  loadStorePass(targetCode);

  // Construct control-flow graph
  CFG cfg;
  buildCFG(targetCode, &cfg);

  // Apply live-range splitter
  //liveRangeSplit(targetCode, &cfg);

  // Perform register allocation
  getSourceTranslate().regAlloc(&cfg, targetCode);

  // Satisfy target code constraints
  satisfy(targetCode);

  // Translate branch-to-labels to relative branches
	debug_break("Do following for vc4 only");
  removeLabels(*targetCode);
}


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

	compileKernel(&m_targetCode, body);
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

	cout << "Errors encountered during encoding:\n";

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
void KernelDriver::pretty(const char *filename) {
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

	print_source_code(f);
	emit_target_code(f);
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
	for (int i = 0; i < m_targetCode.numElems; i++) {
		QPULib::pretty(f, m_targetCode.elems[i], i);
	}
	fprintf(f, "\n");
	fflush(f);
}


}  // namespace QPULib
