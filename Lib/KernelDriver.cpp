#include "KernelDriver.h"
#include <iostream>  // cout
#include "debug.h"

namespace QPULib {

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


}  // namespace QPULib
