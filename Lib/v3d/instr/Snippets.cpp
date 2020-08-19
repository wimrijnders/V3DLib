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

	// This single thread switch and two instructions just before the loop are
	// really important for TMU read to achieve a better performance.
	// This also enables TMU read requests without the thread switch signal, and
	// the eight-depth TMU read request queue.
	ret << nop().thrsw(true)
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

	// This synchronization is needed between the last TMU operation and the
	// program end with the thread switch just before the loop above.
	ret << barrierid(syncb).thrsw(true)
	    << nop()
	    << nop();

	return ret;
}


Instructions end_program() {
	Instructions ret;

	// Program tail
	ret << nop().thrsw(true)
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

