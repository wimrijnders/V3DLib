#include "Snippets.h"
#include "../../Lib/Support/basics.h"

namespace QPULib {
namespace v3d {
namespace instr {


/**
 * An instruction may be passed in to make use of a waiting slot.
 */
Instructions enable_tmu_read(Instr const *last_slot) {
	Instructions ret;

	const char *text = 
		"# This single thread switch and two instructions just before the loop are\n"
		"# really important for TMU read to achieve a better performance.\n"
		"# This also enables TMU read requests without the thread switch signal, and\n"
		"# the eight-depth TMU read request queue.";

	ret << nop().thrsw(true).comment(text)
	    << nop();

	if (last_slot != nullptr) {
		ret << *last_slot;
	} else {
		ret << nop();
	}

	return ret;
}


Instructions sync_tmu() {
	Instructions ret;

	const char *text = 
		"# This synchronization is needed between the last TMU operation and the\n"
		"# program end with the thread switch just before the main body above.";

	ret << barrierid(syncb).thrsw(true).comment(text)
	    << nop()
	    << nop();

	return ret;
}


Instructions end_program() {
	Instructions ret;

	ret << nop().thrsw(true).comment("# Program tail")
	    << nop().thrsw(true)
	    << nop()
	    << nop()
	    << nop().thrsw(true)
	    << nop()
	    << nop()
	    << nop();

	return ret;
}


}  // instr
}  // v3d
}  // QPULib

