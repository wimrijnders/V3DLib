#include "debug.h"
#include <cstdio>
#include "Exception.h"

#ifdef DEBUG

namespace {

enum LogLevel {
	ALL,
	LEVEL_DEBUG,  // DEBUG is a #define
	WARNING,
	NONE
};

LogLevel log_level = ALL;

}  // anon namespace

void debug(const char *str) {
	if (log_level <= LEVEL_DEBUG) {
		printf("DEBUG: %s\n", str);
	}
}


void warning(const char *str) {
	if (log_level <= WARNING) {
		printf("WARNING: %s\n", str);
	}
}


void debug_break(const char *str) {
	if (log_level <= LEVEL_DEBUG) {
		printf("DEBUG: %s\n", str);
		breakpoint
	}
}

#endif  // DEBUG

void disable_logging() {
	log_level = NONE;
}


/**
 * Alternative for `assert` that throws passed string.
 *
 * This is always enabled, also when not building for DEBUG.
 * See header comment of `fatal()` in `basics.h`
 */
void assertq(bool cond, const char *msg, bool do_break) {
	if (cond) {
		return;
	}

	std::string str = "Assertion failed: ";
	str += msg;

#ifdef DEBUG
	if (do_break) {
		breakpoint
	}
#endif

	throw QPULib::Exception(str);
}

