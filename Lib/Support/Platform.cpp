#include "Platform.h"
#include <fstream>
#include <memory>
#include <string.h>  // strstr()
#include "basics.h"

namespace V3DLib {
namespace {

/**
 * @brief read the entire contents of a file into a string
 *
 * @param filename name of file to read
 * @param out_str  output parameter; place to store the file contents
 *
 * @return true if all went well, false if file could not be read.
 */
bool loadFileInString(const char *filename, std::string & out_str) {
  std::ifstream t(filename);
  if (!t.is_open()) {
    return false;
  }

  std::string str((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

  if (str.empty()) {
    return false;
  }

  out_str = str;
  return true;
}


/**
 * @brief Detect Pi platform for newer Pi versions.
 *
 * @param content - output; store platform string here if found
 *
 * @return true if string describing platform found,
 *         false otherwise.
 */
bool get_platform_string(std::string &content) {
  const char *filename = "/sys/firmware/devicetree/base/model";

  bool success = loadFileInString(filename, content);
  if (!success) {
    content == "";
  }

  return success;
}


/**
 * @brief Retrieve the VideoCore chip number.
 *
 * This is the way to detect the Pi platform for older Pi versions/distributions.
 *
 * Detects if this is a VideoCore. This should be sufficient for detecting Pi,
 * since it's the only thing to date(!) using this particular chip version.
 *
 * @return true if Pi detected, false otherwise
 *
 * --------------------------------------------------------------------------
 * ## NOTES
 *
 * * The following are valid model numbers:
 *
 *  - BCM2708
 *  - BCM2835    - This appears to be returned for all higher BCM versions
 *
 * * The following are also valid, but appear to be represented by 'BCM2835'
 *   in `/proc/cpuinfo`:
 *
 *  - BCM2836   // If that's not the case, enable these as well
 *  - BCM2837
 *  - BCM2837B0
 */

bool get_chip_version(std::string &output) {
  const char *BCM_VERSION_PREFIX = "BCM2";
  const char *filename = "/proc/cpuinfo";

  output.clear();

  std::ifstream t(filename);
  if (!t.is_open()) return false;

  std::string line;
  while (getline(t, line)) {
    if (!strstr(line.c_str(), "Hardware")) continue;

    if (strstr(line.c_str(), BCM_VERSION_PREFIX)) {
      // For now, don't try to exactly specify the model.
      // This could be done with field "Revision' in current input.

      size_t pos = line.find(BCM_VERSION_PREFIX);
      if (pos != line.npos) {
        output = line.substr(pos);
      }

      return true;
    }
  }

  return false;
}

// Defined like this to delay the creation of the instance after program init,
// So that other globals get the chance to use it on program init.
std::unique_ptr<PlatformInfo> local_instance;

}  // anon namespace


PlatformInfo::PlatformInfo() {
  is_pi_platform = get_platform_string(platform_id);
  if (get_chip_version(chip_version)) {
    is_pi_platform = true;
  }

  if (!platform_id.empty() && is_pi_platform) {
    has_vc4 = (platform_id.npos == platform_id.find("Pi 4"));
  }

#ifndef QPU_MODE
  // Allow only emulator and interpreter modes, no hardware
  m_use_main_memory = true;
  has_vc4 = true;       // run vc4 code only
#endif
}


void PlatformInfo::output() {
  if (!platform_id.empty()) {
    printf("Platform: %s\n", platform_id.c_str());
  } else {
    printf("Platform: %s\n", "Unknown");
  }

  printf("Chip version: %s\n", chip_version.c_str());

  if (!is_pi_platform) {
    printf("This is NOT a pi platform!\n");
  } else {
    printf("This is a pi platform.\n");

    if (has_vc4) {
      printf("GPU: vc4\n");
    } else {
      printf("GPU: vc6\n");
    }
  }
}


PlatformInfo &Platform::instance_local() {
  if (!local_instance) {
    local_instance.reset(new PlatformInfo);
  }

  return *local_instance;
}


PlatformInfo const  &Platform::instance() {
  return instance_local();
}


void Platform::use_main_memory(bool val) {
#ifdef QPU_MODE
  instance_local().m_use_main_memory = val;
#else
  assertq(instance_local().m_use_main_memory, "Should only use main memory for emulator and interpreter", true);

  if (!val) {
    warning("use_main_memory(): ignoring passed value 'false', because QPU mode is disabled");
  }
#endif
}


/**
 * Sets the target platform to compile to.
 *
 * This is distinct from the platform we are actually running on.
 * The compilation can occur on any platform, including non-pi.
 */
void Platform::compiling_for_vc4(bool val) {
  instance_local().m_compiling_for_vc4 = val;
}


/**
 * Returns the number of available registers in a register file for the current
 * target platform
 *
 * For `vc4`, which has two register files 'A' and 'B' per QPU, returns the size
 * of each register file.
 * `v3d` has one single dual-port register file 'A' per QPU.
 *
 * Current implementation assumes no multi-threading has been enabled on the
 * QPU's. If that ever happens (not likely in this project), the size becomes
 * `size/num_threads`.
 *
 * However, according to internet hearsay, the default and minimum number of
 * threads for `v3d` is actually 2 per QPU. Still, 64 for `v3d` appears to be the
 * correct return value in this case.
 *
 * This all goes to show that something that appears to be exceedingly simple in
 * concept can actually be convoluted as f*** underwater.
 */
int PlatformInfo::size_regfile() const {
  if (m_compiling_for_vc4) {
    return 32;
  } else {
    return 64;
  }
}

}  // namespace V3DLib
