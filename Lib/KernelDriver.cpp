#include "KernelDriver.h"
#include <iostream>  // cout
#include "Support/debug.h"
#include "Source/Syntax.h"  // stmtStack
#include "Source/Pretty.h"
#include "compileKernel.h"
#include "Target/Pretty.h"


namespace QPULib {

void KernelDriver::compile() {
	kernelFinish();

	// Obtain the AST
	body = stmtStack.top();  // stmtStack is a global
	stmtStack.pop();

	// Compile
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
	breakpoint;

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
		fprintf(f, "%i: ", i);
		QPULib::pretty(f, m_targetCode.elems[i]);
	}
	fprintf(f, "\n");
	fflush(f);
}


}  // namespace QPULib
