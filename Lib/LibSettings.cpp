#include "LibSettings.h"
#include "Support/debug.h"

namespace V3DLib {
namespace {
int const QPU_TIMEOUT = 10000;

struct SettingsInternal {
	int qpu_timeout = -1;  // seconds, time to wait for response from QPU
} settings;

}  // anon namespace


/**
 * Get qpu timeout in seconds
 */
int LibSettings::qpu_timeout() {
	if (settings.qpu_timeout == -1) {
		settings.qpu_timeout = QPU_TIMEOUT;
	}

	return settings.qpu_timeout;
}


void LibSettings::qpu_timeout(int val) {
	assert(val > 0);
	assert(settings.qpu_timeout == -1); // For now, allow setting it only once
	settings.qpu_timeout = val;
}


}  // namespace V3DLib
