#include "KernelDriver.h"
#include <iostream>  // cout
#include "Support/debug.h"
#include "Source/Syntax.h"  // stmtStack
#include "Source/Pretty.h"


namespace QPULib {

void KernelDriver::transfer_stack() {
	kernelFinish();

   // Obtain the AST
   body = stmtStack.top();
   stmtStack.pop();
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


}  // namespace QPULib
