#ifdef QPU_MODE

#include "PerformanceCounters.h"
#include <iostream>
#include <sstream>
#include "RegisterMapping.h"
#include "../Support/debug.h"

namespace V3DLib {
namespace v3d {

using RM = RegisterMapping;


const char *PerformanceCounters::Description[PerformanceCounters::NUM_PERF_COUNTERS] = {
	"Unknown 0",
	"Unknown 1",
	"Unknown 2",
	"Unknown 3",
	"Unknown 4",
	"Unknown 5",
	"Unknown 6",
	"Unknown 7",
	"Unknown 8",
	"Unknown 9",
	"Unknown 10",
	"Unknown 11",
	"Unknown 12",
	"Unknown 13",
	"Unknown 14",
	"Unknown 15",
	"Unknown 16",
	"Unknown 17",
	"Unknown 18",
	"Unknown 19",
	"Unknown 20",
	"Unknown 21",
	"Unknown 22",
	"Unknown 23",
	"Unknown 24",
	"Unknown 25",
	"Unknown 26",
	"Unknown 27",
	"Unknown 28",
	"Unknown 29",
	"Unknown 30",
	"Unknown 31",
	"CORE_PCTR_CYCLE_COUNT"
};


void PerformanceCounters::enter(std::vector<int> srcs) {
	auto &regmap = RegisterMapping::instance();
	assert(regmap.info().num_cores == 1);
	int core_id = 0;  // Assuming 1 core with id == 0 sufficient for now

	uint32_t mask = (1 << srcs.size()) - 1;

	regmap.core_write(core_id, RM::CORE_PCTR_0_EN, 0);
	//std::cout << "EN mask before: " << regmap.core_read(core_id, RM::CORE_PCTR_0_EN) << std::endl;

	for (int i = 0; i < srcs.size(); ++i) {
		assert(i < 1);  // TODO adjust for masks
		uint32_t offset = 0;
		uint32_t mask = 127 << offset;

		uint32_t val    = regmap.core_read(core_id, RM::CORE_PCTR_0_SRC_0);
		uint32_t newval = (val & ~mask) + (srcs[i] << offset);
		//std::cout << "newval: " << newval << std::endl;

		regmap.core_write(core_id, RM::CORE_PCTR_0_SRC_0, newval);
	}

	regmap.core_write(core_id, RM::CORE_PCTR_0_CLR     , mask);
	regmap.core_write(core_id, RM::CORE_PCTR_0_OVERFLOW, mask);
	regmap.core_write(core_id, RM::CORE_PCTR_0_EN      , mask);

	//std::cout << "EN mask after: " << regmap.core_read(core_id, RM::CORE_PCTR_0_EN) << std::endl;
}


void PerformanceCounters::exit() {
	auto &regmap = RegisterMapping::instance();
	assert(regmap.info().num_cores == 1);
	int core_id = 0;  // Assuming 1 core with id == 0 sufficient for now

	uint32_t bitMask = regmap.core_read(core_id, RM::CORE_PCTR_0_EN);

	regmap.core_write(core_id, RM::CORE_PCTR_0_EN      , 0);
	regmap.core_write(core_id, RM::CORE_PCTR_0_CLR     , bitMask);
	regmap.core_write(core_id, RM::CORE_PCTR_0_OVERFLOW, bitMask);
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
	//std::cout << "EN mask before: " << bitMask << std::endl;

	std::ostringstream os;
	os << "Enabled counters:\n";

	for (int i = 0; i < SLOT_COUNT; ++i) {
		bool enabled = (0 != (bitMask & (1 << i)));
		if (!enabled) {
			//os << "   Performance counter slot " << i << " not enabled\n";
			continue;
		}
		//os << "   Performance counter slot " << i << " enabled\n";

		assert(i < 1);  // TODO adjust for masks
		uint32_t offset = 0;
		uint32_t mask = 127;

		uint32_t sourceIndex = i;
		uint32_t val = (regmap.core_read(core_id, RM::CORE_PCTR_0_PCTR0) >> offset) & mask;
		uint32_t counterIndex = regmap.core_read(core_id, RM::CORE_PCTR_0_SRC_0);

		if (counterIndex < 0 || counterIndex >= NUM_PERF_COUNTERS) {
			os << "   WARNING: Performance counter index 0x" << std::hex << counterIndex << std::dec
			   << " out of bounds for slot index " << i << "\n";
		} else {
			os << "  " <<  Description[counterIndex] << ": " << val << "\n";
		}
	}

	return os.str();
}


}  // namespace v3d
}  // namespace V3DLib

#endif  // QPU_MODE
