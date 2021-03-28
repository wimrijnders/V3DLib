#ifdef QPU_MODE

#include "PerformanceCounters.h"
#include <iostream>
#include <sstream>
#include "RegisterMapping.h"
#include "../Support/debug.h"

namespace V3DLib {
namespace {

/**
 * Determine the mask and offset to use for given source register.
 *
 * ----------------------------------------------------------------------------
 * Every source offset has four associated masks:
 *
 * Mask is calculated as:  `mask = ((1 << (high - low + 1)) - 1) << low`
 * With (high,low):
 *   - 0: (6, 0)
 *   - 1: (14, 8)
 *   - 2: (22, 16)
 *   - 3: (30, 24)
 */
void get_mask_offset(int source_index, uint32_t &mask, uint32_t &offset) {
  switch (source_index & 3) {
  case 0: mask =        127; offset =  0; break;
  case 1: mask =      32512; offset =  8; break;
  case 2: mask =    8323072; offset = 16; break;
  case 3: mask = 2130706432; offset = 24; break;
  }
}


/**
 * Set the bitfield for given source register with given counter
 */
void set_source_field(int core_id, int source_index, int counter_index) {
  using RM = V3DLib::v3d::RegisterMapping;

#ifdef DEBUG
  using PC = V3DLib::v3d::PerformanceCounters;
  assert(source_index < PC::NUM_SRC_REGS);
#endif  // DEBUG

  auto &regmap = RM::instance();

  uint32_t offset;
  uint32_t mask;
  get_mask_offset(source_index, mask, offset);

  uint32_t src_reg = RM::CORE_PCTR_0_SRC_0 + (source_index >> 2);
  uint32_t val     = regmap.core_read(core_id, src_reg);
  uint32_t newval  = (val & ~mask) | (counter_index << offset);

  regmap.core_write(core_id, src_reg, newval);
}


uint32_t get_source_field(int core_id, int source_index) {
  using RM = V3DLib::v3d::RegisterMapping;

#ifdef DEBUG
  using PC = V3DLib::v3d::PerformanceCounters;
  assert(source_index < PC::NUM_SRC_REGS);
#endif  // DEBUG

  auto &regmap = RM::instance();

  uint32_t offset;
  uint32_t mask;
  get_mask_offset(source_index, mask, offset);

  uint32_t src_reg = RM::CORE_PCTR_0_SRC_0 + (source_index >> 2);
  uint32_t val     = regmap.core_read(core_id, src_reg);
  return (val & mask) >> offset;
}


uint32_t get_pctr_value(int core_id, int source_index) {
  using RM = V3DLib::v3d::RegisterMapping;

#ifdef DEBUG
  using PC = V3DLib::v3d::PerformanceCounters;
  assert(source_index < PC::NUM_SRC_REGS);
#endif  // DEBUG

  auto &regmap = RM::instance();

  return regmap.core_read(core_id, RM::CORE_PCTR_0_PCTR0 + source_index);
}


/*
 Not used, maybe later

void reset_pctr_value(int core_id, int source_index) {
  using RM = V3DLib::v3d::RegisterMapping;
  using PC = V3DLib::v3d::PerformanceCounters;
  assert(source_index < PC::NUM_SRC_REGS);
  auto &regmap = RM::instance();

  regmap.core_write(core_id, RM::CORE_PCTR_0_PCTR0 + source_index, 0);
}
*/

}  // anon namespace


namespace v3d {

using RM = RegisterMapping;


const char *PerformanceCounters::Description[PerformanceCounters::NUM_PERF_COUNTERS] = {
  "Unknown  0 maybe zero",
  "Unknown  1 maybe zero",
  "Unknown  2 maybe zero",
  "Unknown  3 maybe zero",
  "Unknown  4 maybe zero",
  "Unknown  5 maybe zero",
  "Unknown  6 maybe zero",
  "Unknown  7 maybe zero",
  "Unknown  8 maybe zero",
  "Unknown  9 maybe zero",
  "Unknown 10 maybe zero",
  "Unknown 11 maybe zero",
  "Unknown 12 maybe zero",
  "Unknown 13 variable  ",
  "Unknown 14 maybe zero",
  "Unknown 15 maybe zero",
  "Unknown 16 variable  ",
  "Unknown 17 same/app  ",
  "Unknown 18 maybe zero",
  "Unknown 19 maybe zero",
  "Unknown 20 same/app  ",
  "Unknown 21 same/app  ",
  "Unknown 22 same      ",
  "Unknown 23 same      ",
  "Unknown 24 same      ",
  "Unknown 25 same      ",
  "Unknown 26 maybe zero",
  "Unknown 27 maybe zero",
  "Unknown 28 maybe zero",
  "Unknown 29 maybe zero",
  "Unknown 30 same      ",
  "Unknown 31 same      ",
  "CORE_PCTR_CYCLE_COUNT",
  "Unknown 33 maybe zero",
  "Unknown 34 maybe zero",
  "Unknown 35 maybe zero",
  "Unknown 36 fixed     ",
  "Unknown 37 fixed     ",
  "Unknown 38 fixed     ",
  "Unknown 39 fixed     ",
  "Unknown 40 fixed     "
};


/**
 * Assign counters to source registers
 *
 * Counters are assigned sequentially, from source register 0 onwards.
 * The source registers are cleared when an empty param is passed in.
 *
 * @param srcs  list of counter labels to use in source registers
 */
void PerformanceCounters::enter(std::vector<int> srcs) {
  assert(srcs.size() <= NUM_SRC_REGS);
  auto &regmap = RegisterMapping::instance();
  assert(regmap.info().num_cores == 1);
  int core_id = 0;  // Assuming 1 core with id == 0 sufficient for now

  // assign counters to use to source registers
  for (int i = 0; i < (int) srcs.size(); ++i) {
    set_source_field(core_id, i, srcs[i]);
  }

  uint32_t src_mask = (srcs.empty())?0:(1 << srcs.size()) - 1;
  regmap.core_write(core_id, RM::CORE_PCTR_0_CLR     , src_mask);  // Clear selected pctr registers
  regmap.core_write(core_id, RM::CORE_PCTR_0_OVERFLOW, src_mask);
  regmap.core_write(core_id, RM::CORE_PCTR_0_EN      , src_mask);
}


void PerformanceCounters::exit() {
  enter({});
}


/**
 * @brief Create a string representations of the enabled counters and their values.
 */
std::string PerformanceCounters::showEnabled() {
  auto &regmap = RegisterMapping::instance();
  assert(regmap.info().num_cores == 1);
  int core_id = 0;  // Assuming 1 core with id == 0 sufficient for now

  int const SLOT_COUNT = 32;
  uint32_t bitMask = regmap.core_read(core_id, RM::CORE_PCTR_0_EN);

  std::ostringstream os;
  os << "Enabled counters:\n";

  for (int source_index = 0; source_index < SLOT_COUNT; ++source_index) {
    bool enabled = (0 != (bitMask & (1 << source_index)));
    if (!enabled) {
      //os << "   Performance counter slot " << i << " not enabled\n";
      continue;
    }
    //os << "   Performance counter slot " << i << " enabled\n";

    uint32_t counter_index = get_source_field(core_id, source_index);
    uint32_t val           = get_pctr_value(core_id, source_index);

    if (counter_index < 0 || counter_index >= NUM_PERF_COUNTERS) {
      os << "   WARNING: Performance counter 0x" << std::hex << counter_index << std::dec
         << "(" << counter_index << ") "
         << " out of bounds for slot index " << source_index;
      os << ". val: " << val << "\n";
    } else {
      os << "  " <<  Description[counter_index] << ": " << val << "\n";
    }
  }

  return os.str();
}


}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE
