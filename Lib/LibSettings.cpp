#include "LibSettings.h"
#include "Support/basics.h"

namespace V3DLib {
namespace {
int const DEFAULT_HEAP_SIZE = 8*1024*1024;
int const QPU_TIMEOUT = 10;

struct SettingsInternal {
  int  heap_size   = -1;                  // bytes, size of shared (CPU-GPU) memory
  int  qpu_timeout = -1;                  // seconds, time to wait for response from QPU
  bool use_tmu_for_load = true;           // vc4 only, ignored for v3d. If false, use DMA
  bool use_high_precision_sincos = false; // If true, add extra precision to sin/cos calculation for function version
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


int LibSettings::heap_size() {
  if (settings.heap_size == -1) {
    settings.heap_size = DEFAULT_HEAP_SIZE;
  }

  return settings.heap_size;
}


void LibSettings::heap_size(int val) {
  assert(val > 0);
  assert(settings.heap_size == -1); // For now, allow setting it only once
  settings.heap_size = val;
}


bool LibSettings::use_tmu_for_load()         { return settings.use_tmu_for_load; }
void LibSettings::use_tmu_for_load(bool val) { settings.use_tmu_for_load = val; }


bool LibSettings::use_high_precision_sincos()         { return settings.use_high_precision_sincos; }
void LibSettings::use_high_precision_sincos(bool val) { settings.use_high_precision_sincos = val; }

}  // namespace V3DLib
